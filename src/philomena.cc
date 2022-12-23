#include "philomena.h"
#include "json.hpp"
#include <cstdio>
#include <curl/curl.h>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <vector>

using json = nlohmann::json;

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

// Function to make the HTTP GET request and return the response as a string
std::string http_get(const std::string &url) {
  CURL *curl = curl_easy_init();
  std::string response_string;

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      // Handle error
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
                << std::endl;
    }

    curl_easy_cleanup(curl);
  }

  return response_string;
}

void parse_image(const json &json, Image *image) {
  image->wilson_score = json["wilson_score"];
  image->faves = json["faves"];
  if (json["uploader_id"] != nullptr) {
    image->uploader_id = json["uploader_id"];
  }
  image->format = json["format"];
  image->processed = json["processed"];
  image->tags = json["tags"];
  image->id = json["id"];
  image->thumbnails_generated = json["thumbnails_generated"];
  image->name = json["name"];
  image->downvotes = json["downvotes"];
  if (json["deletion_reason"] != nullptr) {
    image->deletion_reason = json["deletion_reason"];
  }
  image->first_seen_at = json["first_seen_at"];
  image->view_url = json["view_url"];
  image->updated_at = json["updated_at"];
  image->created_at = json["created_at"];
  image->orig_sha512_hash = json["orig_sha512_hash"];
  // image->tag_ids = json["tag_ids"];
  image->sha512_hash = json["sha512_hash"];
  image->uploader = json["uploader"];
  if (json["intensities"] != nullptr) {
    image->intensities.ne = json["intensities"]["ne"];
    image->intensities.nw = json["intensities"]["nw"];
    image->intensities.se = json["intensities"]["se"];
    image->intensities.sw = json["intensities"]["sw"];
  }
  image->size = json["size"];
  image->tag_count = json["tag_count"];
  if (json["duplicate_of"] != nullptr) {
    image->duplicate_of = json["duplicate_of"];
  }
  image->animated = json["animated"];
  image->hidden_from_users = json["hidden_from_users"];
  image->comment_count = json["comment_count"];
  image->width = json["width"];
  image->width = json["width"];
  image->representations.full = json["representations"]["full"];
  image->representations.large = json["representations"]["large"];
  image->representations.medium = json["representations"]["medium"];
  image->representations.small = json["representations"]["small"];
  image->representations.tall = json["representations"]["tall"];
  image->representations.thumb = json["representations"]["thumb"];
  image->representations.thumb_small = json["representations"]["thumb_small"];
  image->representations.thumb_tiny = json["representations"]["thumb_tiny"];
  image->spoilered = json["spoilered"];
  image->description = json["description"];
  image->duration = json["duration"];
  image->height = json["height"];
  image->aspect_ratio = json["aspect_ratio"];
  image->upvotes = json["upvotes"];
  image->mime_type = json["mime_type"];
  image->score = json["score"];
  if (json["source_url"] != nullptr) {
    image->source_url = json["source_url"];
  }
}

void parse_image_response(const std::string &json_str,
                          ImageResponse *response) {
  json j = json::parse(json_str);
  parse_image(j["image"], &response->image);
  // response->interactions = j["interactions"];
}

void ImageAPI::get_image(int image_id, ImageResponse &response) {
  // API URL with placeholder for image_id
  std::string url = "https://www.rule34.lol/api/v1/json/images/{image_id}";

  // Replace the placeholder with the actual image_id
  std::string final_url = std::string(url).replace(url.find("{image_id}"), 11,
                                                   std::to_string(image_id));

  // Make the HTTP GET request to the API
  std::string response_string = http_get(final_url);

  parse_image_response(response_string, &response);
}

// Function to convert a given query string to a URL-safe string
std::string to_url_safe_string(const char *query) {
  CURL *curl = curl_easy_init();
  char *output = curl_easy_escape(curl, query, strlen(query));
  curl_easy_cleanup(curl);
  return std::string(output);
}

void parse_query_response(const std::string &json_str,
                          QueryResponse *response) {
  json j = json::parse(json_str);
  for (const auto &image_json : j["images"]) {
    Image image = {};
    parse_image(image_json, &image);
    response->images.push_back(image);
  }
  response->total = j["total"];
}

void ImageAPI::query_images(const std::string &query, long count,
                            QueryResponse *response) {
  // API URL with placeholder for query and count
  std::string url = "https://www.rule34.lol/api/v1/json/search/"
                    "images?sf=random&per_page={count}&q={query}";

  // Replace the placeholders with the actual query and count
  std::string url_safe_query = to_url_safe_string(query.c_str());
  std::cout << "url_safe_query " << url_safe_query << "\n";
  std::string final_url =
      std::string(url).replace(url.find("{query}"), 8, url_safe_query);
  std::cout << "url " << final_url << "\n";
  final_url =
      final_url.replace(final_url.find("{count}"), 7, std::to_string(count));
  std::cout << "url " << final_url << "\n";

  // Make the HTTP GET request to the API
  std::string response_string = http_get(final_url);

  // Parse response
  parse_query_response(response_string, response);
}

std::pair<std::string, int> extract_string_and_int(const std::string &input) {
  std::smatch match;
  std::regex pattern("(.*) \\((\\d+)\\)");

  if (std::regex_match(input, match, pattern)) {
    return std::make_pair(match[1], std::stoi(match[2]));
  } else {
    throw std::invalid_argument("Input does not match expected pattern");
  }
}

// Function that takes a JSON string and parses it into an ApiResponse struct.
void parse_autocomplete_response(const std::string &json_string,
                                 AutocompleteResponse *response) {
  // Parse the JSON string into a JSON object.
  auto root = json::parse(json_string);

  // Iterate over the "entries" array in the JSON.
  for (const auto &entry : root) {
    // Create an Entry object and populate its fields from the JSON object.
    AutocompleteResponse::Entry e;
    e.full = entry["label"].get<std::string>();

    // Parse the value string to extract the value and total fields.
    std::string full = entry["label"].get<std::string>();
    auto tag_and_total = extract_string_and_int(full);
    e.tag = tag_and_total.first;
    e.total = tag_and_total.second;

    // Add the Entry to the vector in the ApiResponse.
    response->entries.push_back(e);
  }
}

void ImageAPI::query_autocomplete(const std::string &query,
                                  AutocompleteResponse *response) {
  // API URL with placeholder for query
  std::string url = "https://www.rule34.lol/autocomplete/tags?term={query}";

  // Replace the placeholder with the actual query
  std::string url_safe_query = to_url_safe_string(query.c_str());
  std::string final_url =
      std::string(url).replace(url.find("{query}"), 8, url_safe_query);

  // Make the HTTP GET request to the API
  std::string response_string = http_get(final_url);

  // Parse response
  parse_autocomplete_response(response_string, response);
}