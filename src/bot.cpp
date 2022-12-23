#include "bot.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "philomena.h"
#include <algorithm>
#include <dpp/dpp.h>
#include <vector>

const std::vector<std::string> &get_no_search_tags() {
  static std::vector<std::string> tags = [] {
    std::vector<std::string> tags = {"loli", "shota", "child"};
    return tags;
  }();
  return tags;
}

const std::vector<std::pair<std::string, std::vector<std::string>>> &
get_fix_words() {
  static std::vector<std::pair<std::string, std::vector<std::string>>> words =
      [] {
        std::vector<std::pair<std::string, std::vector<std::string>>> words = {
            {
                "video",
                {"video game", "video games"},
            },
            {"vid", {}},
            {"video game", {}},
            {"video games", {}},
            {"sex", {}},
            {"gif", {}},
            {"gifs", {}}};
        return words;
      }();
  return words;
}

const std::vector<std::string> &get_rating_tags() {
  static std::vector<std::string> tags = [] {
    std::vector<std::string> tags = {
        "semi-grimdark", "grimdark",     "grotesque", "safe",
        "suggestive",    "questionable", "explicit"};
    return tags;
  }();
  return tags;
}

const std::vector<std::string> &get_ignore_words() {
  static std::vector<std::string> tags = [] {
    std::vector<std::string> tags = {"porn", "rule34"};
    return tags;
  }();
  return tags;
}

int rating_color(std::string rating) {
  using namespace dpp::colors;
  if (rating == "safe") {
    return green;
  } else if (rating == "suggestive") {
    return blue_aquamarine;
  } else if (rating == "questionable") {
    return ruby;
  } else if (rating == "explicit") {
    return scarlet_red;
  } else if (rating == "grimdark") {
    return brown;
  } else if (rating == "grotesque") {
    return cinnabar;
  } else {
    return light_gray;
  }
}

// Reads the content rating tag of the image.
// @param img [Hash] Image data.
// @return [String] Rating tag, or "unknown" if none are present.
std::string rating(const Image &img) {
  for (const auto &tag : img.tags) {
    if (std::find(get_rating_tags().begin(), get_rating_tags().end(), tag) !=
        get_rating_tags().end()) {
      return tag;
    }
  }
  return "unknown";
}

dpp::embed build_image_embed(const Image &image) {
  dpp::embed embed =
      dpp::embed()
          .set_color(rating_color(rating(image)))
          .set_title("https://www.rule34.lol/" + std::to_string(image.id))
          .set_url("https://www.rule34.lol/" + std::to_string(image.id))
          .set_image(image.representations.medium);
  return embed;
}

void handle_image(dpp::cluster &bot, const dpp::slashcommand_t &event,
                  const Image &image) {
  if (image.mime_type == "video/webm") {
    std::string mp4_url = image.representations.medium;
    if (mp4_url.length() >= 4 &&
        mp4_url.substr(mp4_url.length() - 4) == "webm") {
      mp4_url.replace(mp4_url.length() - 4, 4, "mp4");
    }
    bot.message_create(dpp::message(event.command.channel_id, mp4_url)
                           .set_reference(event.command.message_id));
    return;
  }
  dpp::embed embed = build_image_embed(image);

  bot.message_create(dpp::message(event.command.channel_id, embed)
                         .set_reference(event.command.message_id));
}

std::string split_and_join(const std::string &str) {
  // Split the string on commas
  std::vector<absl::string_view> parts = absl::StrSplit(str, ',');

  // Remove any whitespace before and after the commas
  for (auto &part : parts) {
    part = absl::StripLeadingAsciiWhitespace(part);
    part = absl::StripTrailingAsciiWhitespace(part);
  }

  // Join the strings together with commas after
  std::string result;
  bool appended_comma = false;
  for (const auto &part : parts) {
    const std::vector<std::string> &ignore_words = get_ignore_words();
    if (!std::any_of(
            ignore_words.begin(), ignore_words.end(),
            [&](const std::string &ignore) { return ignore == part; })) {
      absl::StrAppend(&result, part, ", ");
      appended_comma = true;
    } else {
      absl::StrAppend(&result, "*, ");
      appended_comma = true;
    }
  }
  if (appended_comma && result.size() > 0) {
    result.pop_back();
    result.pop_back();
  }

  return result;
}

std::string DiscordBot::fix_commas(const std::string &query) {
  std::vector<std::pair<std::string, std::vector<std::string>>> fix_mapping =
      get_fix_words();

  std::vector<std::string> fix_words;
  std::transform(fix_mapping.begin(), fix_mapping.end(),
                 std::back_inserter(fix_words),
                 [](const auto &pair) { return pair.first; });

  // Check if the query includes a space and does not include a comma, and if it
  // contains any fix words
  if (query.find(' ') != std::string::npos &&
      query.find(',') == std::string::npos &&
      std::any_of(fix_words.begin(), fix_words.end(),
                  [&](const std::string &needle) {
                    return query.find(needle) != std::string::npos;
                  })) {
    // Initialize the modified string as a copy of the given string
    std::string new_query = query;

    // Iterate through all fix words
    for (const std::pair<std::string, std::vector<std::string>> &mapping :
         fix_mapping) {
      const std::string &word = mapping.first;
      const std::vector<std::string> &dont_fix = mapping.second;
      // Check for string starting w/ fix word but not dont_fix word
      if (new_query.find(word) == 0 && new_query.length() > word.length() &&
          new_query[word.length()] == ' ') {
        // int pos = new_query.find(word);
        bool dont_fix_mark = false;
        for (const std::string &dont_fix_word : dont_fix) {
          if (new_query.find(dont_fix_word, 0) == 0) {
            dont_fix_mark = true;
          }
        }
        // Replace the fix word with itself followed by a comma and a space
        if (!dont_fix_mark)
          new_query.replace(0, word.length(), word + ",");
      }
      // Check if the modified string ends with the fix
      //     word and does not already have a comma before it
      else if (new_query.rfind(word) != 0 &&
               new_query.rfind(word) == new_query.length() - word.length() &&
               new_query.rfind(", " + word) !=
                   new_query.length() - word.length() - 2) {
        // Replace the fix word with a comma and a space followed by itself
        new_query.replace(new_query.rfind(word), word.length(), "," + word);
      }
    }
    return split_and_join(new_query);
  }
  return split_and_join(query);
}

std::string DiscordBot::fix_query(const std::string &query) {
  std::string new_query = fix_commas(query);
  // Replace "video" w/ ". video" so example "futa video" works

  // Surround with quotes
  if (query.find("(") != std::string::npos &&
      query.find(")") != std::string::npos) {
    new_query = new_query;
  } else {
    new_query = "(" + new_query + ")";
  }
  // no futa
  if (new_query.find("futa") == std::string::npos) {
    new_query += ", -futa";
  }
  // no safe
  if (new_query.find("safe") == std::string::npos) {
    new_query += ", -safe";
  }
  if (new_query.find("!loli") == std::string::npos) {
    const std::vector<std::string> &no_search_tags = get_no_search_tags();
    new_query +=
        ", " + std::accumulate(
                   no_search_tags.begin(), no_search_tags.end(), std::string{},
                   [](auto a, auto b) { return a + "!" + b + ", "; });
    new_query.pop_back();
    new_query.pop_back();
  }
  return new_query;
}

std::optional<std::string>
DiscordBot::expand_single_query(const std::string &query_string) {
  AutocompleteResponse autocomplete_response = {};
  image_api_->query_autocomplete(query_string, &autocomplete_response);

  if (autocomplete_response.entries.size() > 0) {
    std::string new_query = autocomplete_response.entries[0].tag;
    return new_query;
  } else {
    return query_string + "~2";
  }
  return {};
}

// Takes given query, does autocomplete on all substrings and returns new query
// if there are results
std::string DiscordBot::expand_query(const std::string &query_string) {
  std::string expanded_query_string;
  std::string fixed_query = fix_commas(query_string);
  std::vector<absl::string_view> queries = absl::StrSplit(fixed_query, ',');

  // Iterate through the vector of queries and expand each one
  for (const auto &query : queries) {
    // Trim leading and trailing whitespace from the query
    absl::string_view query_view = absl::StripAsciiWhitespace(query);

    // Expand the query
    auto expanded_query = expand_single_query(std::string(query_view));
    if (expanded_query) {
      // Append the expanded query to the output string, separated by a comma
      expanded_query_string += *expanded_query + ",";
    } else {
      // If the expansion failed, append the original query to the output string
      expanded_query_string += std::string(query_view) + ",";
    }
  }

  // Remove the trailing comma from the output string
  if (!expanded_query_string.empty()) {
    expanded_query_string.pop_back();
  }

  return expanded_query_string;
}

void DiscordBot::recursively_query_images(dpp::cluster &bot,
                                          const std::string &original_query,
                                          const std::string &current_query,
                                          long count,
                                          QueryResponse &query_response,
                                          int recurse_level) {
  if (recurse_level > 2) {
    bot.log(dpp::loglevel::ll_info, "LEVEL > 2");
    return;
  }
  bot.log(dpp::loglevel::ll_info, current_query);
  std::string fixed_query = fix_query(current_query);
  bot.log(dpp::loglevel::ll_info, fixed_query);

  QueryResponse temp_query_response;
  image_api_->query_images(fixed_query, count, &temp_query_response);

  bot.log(dpp::loglevel::ll_info,
          "total: " + std::to_string(temp_query_response.total));
  // No images found.
  if (temp_query_response.total == 0) {
    std::string new_query = expand_query(original_query);
    // Don't continue if expanded is same
    if (new_query == original_query) {
      return;
    }
    recursively_query_images(bot, original_query, new_query, count,
                             query_response, recurse_level + 1);
  } else {
    query_response = temp_query_response;
  }
}

void DiscordBot::handle_help(dpp::cluster &bot,
                             const dpp::slashcommand_t &event) {
  event.reply("Just use the slash commands '/picture' or '/video' followed by "
              "a comma-separated query like '/p overwatch, sex'. "
              "https://discord.gg/Gy8zU95TAf");
  // dpp::embed embed =
  //     dpp::embed()
  //         .set_color(dpp::colors::cinnabar)
  //         .set_title("https://www.rule34.lol/" + std::to_string(image.id))
  //         .set_url("https://www.rule34.lol/" + std::to_string(image.id))
  //         .set_image(image.representations.medium);

  // bot.message_create(dpp::message(event.command.channel_id, embed)
  //                        .set_reference(event.command.message_id));
}

void DiscordBot::handle_query(dpp::cluster &bot,
                              const dpp::slashcommand_t &event, bool video) {
  dpp::interaction interaction = event.command;
  auto data = interaction.data;
  dpp::command_interaction cmd_data = interaction.get_command_interaction();

  try {
    const dpp::channel &channel = event.command.get_channel();
    if (!channel.is_nsfw()) {
      event.reply("Please talk to me in a NSFW channel or DMs.");
      return;
    }
  } catch (const dpp::logic_exception &error) {
    // We're in DM so NSFW is fine, so continue.
  }

  // Get query string
  const dpp::command_value &query_value = event.get_parameter("query");

  // Checks if query is empty
  std::string query = (std::holds_alternative<std::monostate>(query_value))
                          ? "*"
                          : std::get<std::string>(query_value);
  if (query == "all" || query == "random") {
    query = "*";
  }
  if (video) {
    query = "video, " + query;
  }

  // Get count string
  const dpp::command_value &count_value = event.get_parameter("count");

  // Checks if count is empty
  long count = (std::holds_alternative<long>(count_value))
                   ? std::min(10L, std::get<long>(count_value))
                   : 3;

  // Get query results
  QueryResponse query_response = {};
  recursively_query_images(bot, query, query, count, query_response);

  std::string response =
      "Found " + std::to_string(query_response.total) + " results.";
  event.reply(response);

  int i = 0;
  for (const Image &image : query_response.images) {
    if (i >= count)
      break;
    handle_image(bot, event, image);
    i++;
  }
}