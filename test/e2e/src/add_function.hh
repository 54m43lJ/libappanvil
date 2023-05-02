#ifndef ADD_FUNCTION_HH
#define ADD_FUNCTION_HH

#include <gtest/gtest.h>

#include "apparmor_parser.hh"
#include "parser/tree/FileNode.hh"

class AddFunctionCheck : public ::testing::Test {
public:
  void SetUp()
  {
    temp_file = ADDITIONAL_PROFILE_SOURCE_DIR "/temp.sd";
  }

  void TearDown()
  {
    std::ignore = std::remove(temp_file.c_str());
  }

protected:
  // File that will be written to and read from temporarily for testing purposes
  std::string temp_file; // NOLINT

  // Adds a file rule to the first profile
  inline void add_file_rule_to_first_profile(AppArmor::Parser &parser, 
                                             const std::string &fileglob,
                                             const std::string &filemode,
                                             std::list<AppArmor::FileRule> &expected_file_rules);

  // Adds a file rule to the last profile
  inline void add_file_rule_to_last_profile(AppArmor::Parser &parser, 
                                            const std::string &fileglob,
                                            const std::string &filemode,
                                            std::list<AppArmor::FileRule> &expected_file_rules);
};

#endif // ADD_FUNCTION_HH