// Copyright 2015 Google Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#include <string>
#include "disk_interface.h"
#include "manifest_parser.h"
#include "state.h"

using std::string;

// Returns predefined data for each file.
struct MockFileReader : public ManifestParser::FileReader {
  MockFileReader(const unsigned char *data, size_t size)
      : data_(data), size_(size) {}

  virtual bool ReadFile(const string& path, string* content, string* err) {
    content->assign(reinterpret_cast<const char*>(data_), size_);
    // Only return data for the main manifest, else every include leads
    // to an infinite include stack.
    size_ = 0;
    return true;
  }

  const unsigned char* data_;
  size_t size_;
};

extern "C" int LLVMFuzzerTestOneInput(const unsigned char *data, size_t size) {
  if (size < 1)
    return 0;

  MockFileReader file_reader(data, size);

  State state;

  // Make dupe edges errors not warnings.  That way they go into |err| below
  // instead of to stderr -- writing to stderr slows down fuzzing.
  ManifestParser parser(&state, &file_reader, /*dupe_edge_should_err=*/true);

  string err;
  parser.Load("foo.ninja", &err);
  return 0;
}
