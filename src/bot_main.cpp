#include "bot.h"
#include "philomena.h"
#include <dpp/dpp.h>

const std::string DISCORD_BOT_TOKEN = std::getenv("DISCORD_BOT_TOKEN");

int main() {
  dpp::cluster bot(DISCORD_BOT_TOKEN);
  ImageAPI image_api;
  VirtualImageApi *virtual_image_api = &image_api;
  DiscordBot discord_bot(virtual_image_api);

  bot.on_log(dpp::utility::cout_logger());

  bot.on_slashcommand([&bot, &discord_bot](const dpp::slashcommand_t &event) {
    // event.thinking(true);
    const std::string &command_name = event.command.get_command_name();
    if (command_name == "picture" || command_name == "p") {
      discord_bot.handle_query(bot, event);
    } else if (command_name == "video" || command_name == "v") {
      discord_bot.handle_query(bot, event, /*video=*/true);
    } else if (command_name == "help") {
      discord_bot.handle_help(bot, event);
    } else {
      event.reply("???");
    }
  });

  bot.on_ready([&bot](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      dpp::command_completion_event_t callback = dpp::utility::log_error();
      dpp::slashcommand command(
          "picture",
          "Queries the bot for porn. Example: /picture overwatch, sex",
          bot.me.id);
      command.set_dm_permission(true);
      command.add_option(dpp::command_option(dpp::co_string, "query",
                                             "The query you're using", true));
      command.add_option(dpp::command_option(
          dpp::co_integer, "count", "# of images you want (max 10)", false));
      bot.global_command_create(command, callback);

      dpp::slashcommand command2(
          "p", "Queries the bot for porn. Example: /p overwatch, sex",
          bot.me.id);
      command2.set_dm_permission(true);
      command2.add_option(dpp::command_option(dpp::co_string, "query",
                                              "The query you're using", true));
      command2.add_option(dpp::command_option(
          dpp::co_integer, "count", "# of images you want (max 10)", false));

      bot.global_command_create(command2, callback);

      dpp::slashcommand command3(
          "video", "Queries the bot for porn. Example: /video overwatch, sex",
          bot.me.id);
      command3.set_dm_permission(true);
      command3.add_option(dpp::command_option(dpp::co_string, "query",
                                              "The query you're using", true));
      command3.add_option(dpp::command_option(
          dpp::co_integer, "count", "# of images you want (max 10)", false));
      bot.global_command_create(command3, callback);

      dpp::slashcommand command4(
          "v", "Queries the bot for porn. Example: /v overwatch, sex",
          bot.me.id);
      command4.set_dm_permission(true);
      command4.add_option(dpp::command_option(dpp::co_string, "query",
                                              "The query you're using", true)
                              .set_max_value(10));
      command4.add_option(dpp::command_option(
          dpp::co_integer, "count", "# of images you want (max 10)", false));
      bot.global_command_create(command4, callback);

      dpp::slashcommand command5("help", "Help message!", bot.me.id);
      command5.set_dm_permission(true);
      bot.global_command_create(command5, callback);
    }
  });

  bot.start(dpp::st_wait);
}
