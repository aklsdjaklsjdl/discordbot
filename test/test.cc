#include "../src/bot.h"
#include "../src/philomena.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using json = nlohmann::json;
using testing::_;
using testing::Invoke;
using testing::Return;

class MockImageAPI : public VirtualImageApi {
public:
  MOCK_METHOD(void, get_image, (int image_id, ImageResponse &response),
              (override));
  MOCK_METHOD(void, query_images,
              (const std::string &query, long count, QueryResponse *response),
              (override));
  MOCK_METHOD(void, query_autocomplete,
              (const std::string &query, AutocompleteResponse *response),
              (override));
};

class PhilomenaTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void mock_autocomplete(const std::string &query,
                         const AutocompleteResponse &expected_response) {
    EXPECT_CALL(mock_api_, query_autocomplete(query, _))
        .WillRepeatedly(
            Invoke([&expected_response](const std::string &query,
                                        AutocompleteResponse *response) {
              *response = expected_response;
            }));
  }

  AutocompleteResponse get_autocomplete_response(const std::string &query) {
    std::string json_str = "[]";
    if (query == "anal se") {
      json_str = R"#([
        {"label":"anal sex (9138)","value":"anal sex"},
        {"label":"anal squirt (9)","value":"anal squirt"},
        {"label":"anal spitroast (8)","value":"anal spitroast"},
        {"label":"anal squirting (7)","value":"anal squirting"},
        {"label":"anal stretching (6)","value":"anal stretching"}])#";
    } else if (query == "naruto") {
      json_str = R"#([
        {"label":"naruto (series) (3134)","value":"naruto (series)"},
        {"label":"naruto shippuden (819)","value":"naruto shippuden"},
        {"label":"naruto uzumaki (651)","value":"naruto uzumaki"},
        {"label":"naruto: the last (112)","value":"naruto: the last"},
        {"label":"naruto shippuuden (108)","value":"naruto shippuuden"}])#";
    } else if (query == "vid") {
      json_str = R"#([
        {"label":"video (15572)","value":"video"},
        {"label":"video games (9074)","value":"video games"},
        {"label":"video game (276)","value":"video game"},
        {"label":"videl (239)","value":"videl"},
        {"label":"video game character (84)","value":"video game character"}])#";
    }
    AutocompleteResponse response = {};
    parse_autocomplete_response(json_str, &response);
    return response;
  }

  MockImageAPI mock_api_;
  DiscordBot bot_{&mock_api_};
};

TEST_F(PhilomenaTest, Commas) {
  EXPECT_EQ(bot_.fix_commas("futa"), "futa");
  EXPECT_EQ(bot_.fix_commas("video"), "video");
  EXPECT_EQ(bot_.fix_commas("futa video"), "futa, video");
  EXPECT_EQ(bot_.fix_commas("futa, video"), "futa, video");
  EXPECT_EQ(bot_.fix_commas("raven video"), "raven, video");
  EXPECT_EQ(bot_.fix_commas("video games"), "video games");
  EXPECT_EQ(bot_.fix_commas("video game sex"), "video game, sex");
  EXPECT_EQ(bot_.fix_commas("video games sex"), "video games, sex");
  EXPECT_EQ(bot_.fix_commas("video video games"), "video, video games");
  EXPECT_EQ(bot_.fix_commas("video games video"), "video games, video");
  // EXPECT_EQ(bot_.fix_commas("sex video games penis"), "sex, video games,
  // penis"); this is hard
}

TEST_F(PhilomenaTest, AutocompleteParse) {
  auto tag_and_total = extract_string_and_int("naruto (series) (1234)");
  EXPECT_EQ(tag_and_total.first, "naruto (series)");
  EXPECT_EQ(tag_and_total.second, 1234);
}

TEST_F(PhilomenaTest, IgnoreWords) {
  EXPECT_EQ(bot_.fix_commas("porn"), "*");
  EXPECT_EQ(bot_.fix_commas("rule34"), "*");
}

TEST_F(PhilomenaTest, Autocomplete) {
  std::string query = "anal se";
  AutocompleteResponse expected_response = get_autocomplete_response(query);
  mock_autocomplete(query, expected_response);

  // Call the mock method and verify the response
  AutocompleteResponse response = {};
  mock_api_.query_autocomplete(query, &response);
  EXPECT_EQ(response.entries.size(), 5);
  EXPECT_EQ(response.entries[0].total, 9138);
  EXPECT_EQ(response.entries[0].full, "anal sex (9138)");
  EXPECT_EQ(response.entries[0].tag, "anal sex");
}

TEST_F(PhilomenaTest, AutocompleteExpand) {
  std::string query = "naruto";
  AutocompleteResponse expected_response = get_autocomplete_response(query);
  mock_autocomplete(query, expected_response);

  std::string query2 = "vid";
  AutocompleteResponse expected_response2 = get_autocomplete_response(query2);
  mock_autocomplete(query2, expected_response2);

  EXPECT_EQ(bot_.expand_single_query("naruto"), "naruto (series)");
  EXPECT_EQ(bot_.expand_single_query("vid"), "video");
  EXPECT_EQ(bot_.expand_query("naruto vid"), "naruto (series),video");
}

TEST_F(PhilomenaTest, AutocompleteExpandAndFuzz) {
  std::string query = "micacfrt";
  AutocompleteResponse expected_response = get_autocomplete_response(query);
  mock_autocomplete(query, expected_response);

  std::string query2 = "vid";
  AutocompleteResponse expected_response2 = get_autocomplete_response(query2);
  mock_autocomplete(query2, expected_response2);

  std::string full_query = "micacfrt vid";
  EXPECT_EQ(bot_.expand_single_query("vid"), "video");
  EXPECT_EQ(bot_.expand_query(full_query), "micacfrt~2,video");
}

TEST_F(PhilomenaTest, QueryResponse) {
  std::string suggestive_response_json = R"#(
    {"images":[{"wilson_score":0,"faves":0,"uploader_id":1,"format":"png","processed":true,"tags":["female","breasts","fox","solo female","suggestive","artist:persimon"],"id":214555,"thumbnails_generated":true,"name":"001071.1269764391.png","downvotes":0,"deletion_reason":null,"first_seen_at":"2022-09-10T15:38:29Z","view_url":"https://cdn.rule34.lol/img/view/2022/9/10/214555__artist-colon-persimon_breasts_female_fox_solo+female_suggestive.png","updated_at":"2022-09-10T15:38:37Z","created_at":"2022-09-10T15:38:29Z","orig_sha512_hash":"6c2ec4f84c9e22f70fb90a78e5eae658fc8483fa81878162547c5bb2200f1c681060782d9a211cd9aeafeeacc32eabf6057d70945c96fefebc5c5a127c0f32c0","tag_ids":[15,261,415,2768,10926,90885],"sha512_hash":"7c1d070dd2f04e501c761f599821505f5fbb7de1c86a68aed12f842a77fc2659e35d23f94e66a3e2a75b010d7135dbd0a3311356eb1a73366dab829900151eea","uploader":"Administrator","intensities":{"ne":123.805144,"nw":133.904021,"se":105.736918,"sw":126.858574},"size":400275,"tag_count":6,"duplicate_of":null,"animated":false,"hidden_from_users":false,"comment_count":0,"width":512,"representations":{"full":"https://cdn.rule34.lol/img/view/2022/9/10/214555.png","large":"https://cdn.rule34.lol/img/2022/9/10/214555/full.png","medium":"https://cdn.rule34.lol/img/2022/9/10/214555/medium.png","small":"https://cdn.rule34.lol/img/2022/9/10/214555/small.png","tall":"https://cdn.rule34.lol/img/2022/9/10/214555/full.png","thumb":"https://cdn.rule34.lol/img/2022/9/10/214555/thumb.png","thumb_small":"https://cdn.rule34.lol/img/2022/9/10/214555/thumb_small.png","thumb_tiny":"https://cdn.rule34.lol/img/2022/9/10/214555/thumb_tiny.png"},"spoilered":false,"description":"","duration":0.04,"height":768,"aspect_ratio":0.6666666666666666,"upvotes":0,"mime_type":"image/png","score":0,"source_url":null},{"wilson_score":0,"faves":0,"uploader_id":1,"format":"jpg","processed":true,"tags":["female","simple background","hi res","mammal","anthro","solo","food","clothing","condom wrapper","artist:loimu","loimu (character)","cervid","2020","uniform","young","colored nails","nails","furry","teenager","school uniform","pink eyes","suggestive","pocky"],"id":150304,"thumbnails_generated":true,"name":"b4da4f8b2dab7ccfd34386bd2185f2dc.jpg","downvotes":0,"deletion_reason":null,"first_seen_at":"2022-01-14T04:52:39Z","view_url":"https://cdn.rule34.lol/img/view/2022/1/14/150304__artist-colon-loimu_anthro_2020_cervid_clothing_colored+nails_condom+wrapper_female_food_furry_hi+res_loimu+character_mammal_nails_pink+eyes_pocky_schoo.jpg","updated_at":"2022-01-14T04:58:41Z","created_at":"2022-01-14T04:52:39Z","orig_sha512_hash":"37acfc0c43787d7a94b573731ba73ef227c146da0bccbf2879da78f5a02116089e2daa5a7af76bdf043d046f4256ff1efb378f0466e83d5503bf3bd107569305","tag_ids":[15,102,182,186,201,285,309,399,495,552,553,567,941,1546,1941,1979,1983,2517,2772,3261,3696,10926,24379],"sha512_hash":"30efdd45b595076f88620ffa4ccd6bec1c9ecb068c2001f2067809081b237d46150c9b320b336b3a4669e3eb5c37de01f5e1024c7f8d9c08636f5ba322f0c563","uploader":"Administrator","intensities":{"ne":228.296718,"nw":210.097978,"se":180.454677,"sw":187.529269},"size":280002,"tag_count":23,"duplicate_of":null,"animated":false,"hidden_from_users":false,"comment_count":0,"width":1267,"representations":{"full":"https://cdn.rule34.lol/img/view/2022/1/14/150304.jpg","large":"https://cdn.rule34.lol/img/2022/1/14/150304/large.jpg","medium":"https://cdn.rule34.lol/img/2022/1/14/150304/medium.jpg","small":"https://cdn.rule34.lol/img/2022/1/14/150304/small.jpg","tall":"https://cdn.rule34.lol/img/2022/1/14/150304/tall.jpg","thumb":"https://cdn.rule34.lol/img/2022/1/14/150304/thumb.jpg","thumb_small":"https://cdn.rule34.lol/img/2022/1/14/150304/thumb_small.jpg","thumb_tiny":"https://cdn.rule34.lol/img/2022/1/14/150304/thumb_tiny.jpg"},"spoilered":false,"description":"","duration":0.04,"height":1280,"aspect_ratio":0.98984375,"upvotes":0,"mime_type":"image/jpeg","score":0,"source_url":"https://e621.net/posts/2517177"},{"wilson_score":0,"faves":0,"uploader_id":1,"format":"png","processed":true,"tags":["text","clothed","hair","red eyes","1boy","1girl","1boy1girl","skirt","white shirt","text bubble","skullgirls","suggestive","artist:captain kirb","painwheel"],"id":143278,"thumbnails_generated":true,"name":"966e50dcb03d235539346aa4c1f7235c.png?5153860","downvotes":0,"deletion_reason":null,"first_seen_at":"2022-01-12T23:46:50Z","view_url":"https://cdn.rule34.lol/img/view/2022/1/12/143278__artist-colon-captain+kirb_1boy_1boy1girl_1girl_clothed_hair_painwheel_red+eyes_skirt_skullgirls_suggestive_text_text+bubble_white+shirt.png","updated_at":"2022-01-12T23:50:51Z","created_at":"2022-01-12T23:46:50Z","orig_sha512_hash":"03644db5f48f9840f49f5716949f4f9211ddc2d8ce58d8c7736a801bb20cdde94c01dc27cae668bf176c481cfb6cf91258a7e128441c2380c0219e34b2af5c24","tag_ids":[286,398,685,1182,2489,2539,2589,2897,3624,4932,6064,10926,14910,18377],"sha512_hash":"bdca74b0ed57a215c4c49f1a6d0d6a16e3c10b496fb712edd051ccb984b0f510334acc75234a2c9a5010df02f021c8a82d8de75a2fea42ca6796a7e6b69cb1f6","uploader":"Administrator","intensities":{"ne":41.844564,"nw":53.149533,"se":48.499142,"sw":56.755238},"size":501671,"tag_count":14,"duplicate_of":null,"animated":false,"hidden_from_users":false,"comment_count":0,"width":1285,"representations":{"full":"https://cdn.rule34.lol/img/view/2022/1/12/143278.png","large":"https://cdn.rule34.lol/img/2022/1/12/143278/large.png","medium":"https://cdn.rule34.lol/img/2022/1/12/143278/medium.png","small":"https://cdn.rule34.lol/img/2022/1/12/143278/small.png","tall":"https://cdn.rule34.lol/img/2022/1/12/143278/tall.png","thumb":"https://cdn.rule34.lol/img/2022/1/12/143278/thumb.png","thumb_small":"https://cdn.rule34.lol/img/2022/1/12/143278/thumb_small.png","thumb_tiny":"https://cdn.rule34.lol/img/2022/1/12/143278/thumb_tiny.png"},"spoilered":false,"description":"","duration":0.04,"height":1613,"aspect_ratio":0.796652200867948,"upvotes":0,"mime_type":"image/png","score":0,"source_url":"https://rule34.xxx/index.php?page=post&s=view&id=5153860"}],"interactions":[],"total":5}
  )#";
  QueryResponse response = {};
  parse_query_response(suggestive_response_json, &response);
  EXPECT_EQ(response.total, 5);
}