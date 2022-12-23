#ifndef PHILOMENA_H_NAME
#define PHILOMENA_H_NAME

#include "json.hpp"
#include <cstdio>
#include <curl/curl.h>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

struct Image {
  double wilson_score;
  int faves;
  std::optional<int> uploader_id;
  std::string format;
  bool processed;
  std::vector<std::string> tags;
  int id;
  bool thumbnails_generated;
  std::string name;
  int downvotes;
  std::optional<std::string> deletion_reason;
  std::string first_seen_at;
  std::string view_url;
  std::string updated_at;
  std::string created_at;
  std::string orig_sha512_hash;
  std::vector<int> tag_ids;
  std::string sha512_hash;
  std::string uploader;
  struct Intensities { // can be null
    double ne;
    double nw;
    double se;
    double sw;
  } intensities;
  int size;
  int tag_count;
  int duplicate_of;
  bool animated;
  bool hidden_from_users;
  int comment_count;
  int width;
  struct Representations {
    std::string full;
    std::string large;
    std::string medium;
    std::string small;
    std::string tall;
    std::string thumb;
    std::string thumb_small;
    std::string thumb_tiny;
  } representations;
  bool spoilered;
  std::string description;
  double duration;
  int height;
  double aspect_ratio;
  int upvotes;
  std::string mime_type;
  int score;
  std::optional<std::string> source_url;
};

struct ImageResponse {
  Image image;
  // std::vector<int> interactions;
};

struct QueryResponse {
  std::vector<Image> images;
  int total;
};

struct AutocompleteResponse {
  struct Entry {
    std::string full;
    std::string tag;
    int total;
  };
  std::vector<Entry> entries;
};

class VirtualImageApi {
public:
  virtual ~VirtualImageApi() {}
  virtual void get_image(int image_id, ImageResponse &response) = 0;
  virtual void query_images(const std::string &query, long count,
                            QueryResponse *response) = 0;
  virtual void query_autocomplete(const std::string &query,
                                  AutocompleteResponse *response) = 0;
};

class ImageAPI : public VirtualImageApi {
public:
  void get_image(int image_id, ImageResponse &response);
  void query_images(const std::string &query, long count,
                    QueryResponse *response);
  void query_autocomplete(const std::string &query,
                          AutocompleteResponse *response);
};

void parse_autocomplete_response(const std::string &json_string,
                                 AutocompleteResponse *response);
void parse_query_response(const std::string &json_str, QueryResponse *response);
std::pair<std::string, int> extract_string_and_int(const std::string &input);

#endif