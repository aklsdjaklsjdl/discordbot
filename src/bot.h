#ifndef DISCORD_BOT_H_NAME
#define DISCORD_BOT_H_NAME

#include "philomena.h"
#include <dpp/dpp.h>

class DiscordBot {
public:
  DiscordBot(VirtualImageApi *image_api) : image_api_(image_api) {}
  void handle_query(dpp::cluster &bot, const dpp::slashcommand_t &event,
                    bool video = false);
  std::string fix_commas(const std::string &query);
  std::string fix_query(const std::string &query);
  std::optional<std::string>
  expand_single_query(const std::string &query_string);
  std::string expand_query(const std::string &query_string);
  void handle_help(dpp::cluster &bot, const dpp::slashcommand_t &event);

private:
  void recursively_query_images(dpp::cluster &bot,
                                const std::string &original_query,
                                const std::string &current_query, long count,
                                QueryResponse &query_response,
                                int recurse_level = 0);
  VirtualImageApi *image_api_;
};

#endif