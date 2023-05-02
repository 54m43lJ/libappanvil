#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_set>

#include "apparmor_file_rule.hh"
#include "apparmor_parser.hh"
#include "apparmor_profile.hh"
#include "parser/tree/FileNode.hh"

namespace RemoveFunctionCheck {
    std::list<AppArmor::Profile> getProfileList(const std::string &filename)
    {
        AppArmor::Parser parser(filename);
        return parser.getProfileList();
    }

    void check_file_rules_for_single_profile(const std::string &filename,
                                             const std::list<AppArmor::FileRule> &expected_file_rules,
                                             const std::string &profile_name)
    {
        auto profile_list = getProfileList(filename);
        while(profile_name != profile_list.front().name() && !profile_list.empty()){
            profile_list.pop_front();
        }

        auto profile = profile_list.front();
        EXPECT_EQ(profile.name(), profile_name) << "No profile name matched";

        auto file_rules = profile.getFileRules();
        ASSERT_EQ(file_rules, expected_file_rules);
    }

    void emplace_back(std::list<AppArmor::FileRule> &list, const std::string &filename, const std::string &filemode)
    {
        FileNode node(0, 1, filename, filemode);
        auto node_pointer = std::make_shared<FileNode>(node);
        AppArmor::FileRule rule(node_pointer);
        list.emplace_back(rule);
    }

    //Test to remove a rule from a file with 1 profile and 1 rule
    TEST(RemoveFunctionCheck, test1_remove) // NOLINT
    {
        std::string filename = ADDITIONAL_PROFILE_SOURCE_DIR "/remove-untouched/test1_remove.sd";

        std::list<AppArmor::FileRule> expected_file_rules;

        //remove rule /usr/X11R6/lib/lib*so* rrr,
        AppArmor::Parser parser(filename);

        auto profile_list = parser.getProfileList();
        ASSERT_FALSE(profile_list.empty()) << "There should be at least one profile";
        auto prof = profile_list.front();

        auto rule_list = prof.getFileRules();
        ASSERT_FALSE(rule_list.empty()) << "There should be at least one file rule";
        auto frule = rule_list.front();

        parser = parser.removeRule(prof, frule);

        check_file_rules_for_single_profile(filename, expected_file_rules, "/**");
    }

    //Test to remove a rule from a file with 1 profile and more than 1 rule
    TEST(RemoveFunctionCheck, test2_remove) // NOLINT
    {
        std::string filename = ADDITIONAL_PROFILE_SOURCE_DIR "/remove-untouched/test2_remove.sd";
        std::list<AppArmor::FileRule> expected_file_rules;

        emplace_back(expected_file_rules, "/does/not/exist", "r");
        emplace_back(expected_file_rules, "/var/log/messages", "www");

        AppArmor::Parser parser(filename);

        auto profile_list = parser.getProfileList();
        ASSERT_FALSE(profile_list.empty()) << "There should be at least one profile";
        auto prof = profile_list.front();

        auto rule_list = prof.getFileRules();
        ASSERT_FALSE(rule_list.empty()) << "There should be at least one file rule";
        auto frule = rule_list.front();

        parser = parser.removeRule(prof, frule);

        check_file_rules_for_single_profile(filename, expected_file_rules, "/**");
    }

    //Test to remove a rule from a file with 2 profiles and 1 rule each
    TEST(RemoveFunctionCheck, test3_remove) // NOLINT
    {
        std::string filename = ADDITIONAL_PROFILE_SOURCE_DIR "/remove-untouched/test3_remove.sd";
        std::list<AppArmor::FileRule> expected_file_rules1;
        std::list<AppArmor::FileRule> expected_file_rules2;

        emplace_back(expected_file_rules2, "/usr/X11R6/lib/lib*so*", "rrr");

        AppArmor::Parser parser(filename);

        auto profile_list = parser.getProfileList();
        ASSERT_FALSE(profile_list.empty()) << "There should be at least one profile";
        auto prof = profile_list.front();

        auto rule_list = prof.getFileRules();
        ASSERT_FALSE(rule_list.empty()) << "There should be at least one file rule";
        auto frule = rule_list.front();

        parser = parser.removeRule(prof, frule);

        check_file_rules_for_single_profile(filename, expected_file_rules1, "/**");
        check_file_rules_for_single_profile(filename, expected_file_rules2, "/*");
    }

    //Test to remove a rule from a file with 2 profiles and more than 1 rule each
    TEST(RemoveFunctionCheck, test4_remove) // NOLINT
    {
        std::string filename = ADDITIONAL_PROFILE_SOURCE_DIR "/remove-untouched/test4_remove.sd";
        std::list<AppArmor::FileRule> expected_file_rules1;
        std::list<AppArmor::FileRule> expected_file_rules2;

        emplace_back(expected_file_rules1, "/does/not/exist", "r");
        emplace_back(expected_file_rules1, "/var/log/messages", "www");

        emplace_back(expected_file_rules2, "/usr/X11R6/lib/lib*so*", "rrr");
        emplace_back(expected_file_rules2, "/does/not/exist", "r");
        emplace_back(expected_file_rules2, "/var/log/messages", "www");

        AppArmor::Parser parser(filename);

        auto profile_list = parser.getProfileList();
        ASSERT_FALSE(profile_list.empty()) << "There should be at least one profile";
        auto prof = profile_list.front();

        auto rule_list = prof.getFileRules();
        ASSERT_FALSE(rule_list.empty()) << "There should be at least one file rule";
        auto frule = rule_list.front();

        parser = parser.removeRule(prof, frule);

        check_file_rules_for_single_profile(filename, expected_file_rules1, "/**");
        check_file_rules_for_single_profile(filename, expected_file_rules2, "/*");
    }

    // //Test to remove a rule that DNI from a file with 1 profile and 1 rule
    // TEST(RemoveFunctionCheck, test5)
    // {
    //     std::string filename = ADDITIONAL_PROFILE_SOURCE_DIR "/remove-untouched/test5.sd";
    //     std::list<AppArmor::FileRule> expected_file_rules;

    //     emplace_back(expected_file_rules, /usr/X11R6/lib/lib*so*, rrr);

    //     //remove nonexistant rule from profile /**

    //     check_file_rules_for_single_profile(filename, expected_file_rules, "/**");
    // }

    // //Test to remove a rule from a profile that DNI from a file with 1 profile and 1 rule
    // TEST(RemoveFunctionCheck, test6)
    // {
    //     std::string filename = ADDITIONAL_PROFILE_SOURCE_DIR "/remove-untouched/test6.sd";
    //     std::list<AppArmor::FileRule> expected_file_rules;

    //     emplace_back(expected_file_rules, /usr/X11R6/lib/lib*so*, rrr);

    //     //remove rule /usr/X11R6/lib/lib*so* rrr, from nonexistant profile

    //     check_file_rules_for_single_profile(filename, expected_file_rules, "/**");
    // }
} // namespace RemoveFunctionCheck