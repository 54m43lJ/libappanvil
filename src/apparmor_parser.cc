#include "apparmor_parser.hh"
#include "apparmor_profile.hh"
#include "parser/driver.hh"
#include "parser/lexer.hh"
#include "parser/tree/ParseTree.hh"

#include <fstream>
#include <memory>
#include <parser_yacc.hh>
#include <stdexcept>
#include <string>

AppArmor::Parser::Parser(const std::string &path)
  : path{path}
{
    std::ifstream stream(path);

    // Read entire file into string and store if for later
    std::stringstream ss;
    ss << stream.rdbuf();
    file_contents = ss.str();

    // Seek back to beginning of file
    stream.seekg(0);

    // Parse the file contents
    update_from_stream(stream);
}

void AppArmor::Parser::update_from_file_contents()
{
    // Put the file contents into a stream
    std::stringstream stream;
    stream << file_contents;
    update_from_stream(stream);
}

void AppArmor::Parser::update_from_stream(std::istream &stream)
{
    // Perform lexical analysis
    Lexer lexer(stream, std::cerr);

    // Parse the file
    Driver driver;
    yy::parser parse(lexer, driver);
    parse();

    // If parsing was not successful, throw an exception
    if(!driver.success) {
        std::throw_with_nested(std::runtime_error("error occured when parsing profile"));
    }

    // Create or update the list of profiles
    initializeProfileList(driver.ast);
}

void AppArmor::Parser::initializeProfileList(const std::shared_ptr<ParseTree> &ast)
{
    profile_list = std::list<Profile>();
    
    auto astList = ast->profileList;
    for (auto prof_iter = astList->begin(); prof_iter != astList->end(); prof_iter++){
        std::shared_ptr<ProfileNode> node = std::make_shared<ProfileNode>(*prof_iter);
        Profile profile(node);
        profile_list.push_back(profile);
    }
}

std::list<AppArmor::Profile> AppArmor::Parser::getProfileList() const
{
    return profile_list;
}

void AppArmor::Parser::checkProfileValid(AppArmor::Profile &profile)
{
    // Attempt to find profile from the list and return on success
    for(const AppArmor::Profile &prof : profile_list) {
        if(profile == prof) {
            return;
        }
    }

    // Profile was not found so throw an exception
    std::stringstream message;
    message << "Invalid profile \"" << profile.name() << "\" was given as argument. This profile does not exist in this parser. Was it created using a different or outdated AppArmor::Parser object?\n";
    throw std::domain_error(message.str());
}

void AppArmor::Parser::removeRule(AppArmor::Profile &profile, AppArmor::FileRule &fileRule)
{
    std::ofstream output_file(path);
    removeRule(profile, fileRule, output_file);
    output_file.close();
}

void AppArmor::Parser::removeRule(AppArmor::Profile &profile, AppArmor::FileRule &fileRule, std::ostream &output)
{
    checkProfileValid(profile);
    profile.checkFileRuleValid(fileRule);

    // Erase the fileRule from 'file_contents'
    auto start_pos = static_cast<uint>(fileRule.getStartPosition()) - 1;
    auto end_pos   = fileRule.getEndPosition();
    auto length    = end_pos - start_pos;

    file_contents.erase(start_pos, length);

    // Push changes to 'output_file' and update changes
    output << file_contents;
    update_from_file_contents();
}

void AppArmor::Parser::addRule(Profile &profile, const std::string &fileglob, const std::string &filemode)
{
    std::ofstream output_file(path);
    addRule(profile, fileglob, filemode, output_file);
    output_file.close();
}

void AppArmor::Parser::addRule(Profile &profile, const std::string &fileglob, const std::string &filemode, std::ostream &output)
{
    checkProfileValid(profile);

    // Get the position of the last rule
    auto pos = profile.getRuleEndPosition();

    // Create and insert the rule (TODO: Fix possible invalid rules and injection of extra rules)
    std::string addRule = "  " + fileglob + " " + filemode + ",\n";
    file_contents.insert(pos, addRule);

    // Push changes to 'output_file' and update changes
    output << file_contents;
    update_from_file_contents();
}

void AppArmor::Parser::editRule(Profile &profile,
                                FileRule &oldRule,
                                const std::string &fileglob,
                                const std::string &filemode)
{
    std::ofstream output_file(path);
    editRule(profile, oldRule, fileglob, filemode, output_file);
    output_file.close();
}

void AppArmor::Parser::editRule(Profile &profile,
                                FileRule &oldRule,
                                const std::string &fileglob,
                                const std::string &filemode,
                                std::ostream &output)
{
    checkProfileValid(profile);
    profile.checkFileRuleValid(oldRule);

    // Remove and replace the fileRule from 'file_contents'
    auto start_pos = oldRule.getStartPosition() - 1;
    auto end_pos   = oldRule.getEndPosition();
    auto length    = end_pos - start_pos;

    // Remove the old rule
    file_contents.erase(start_pos, length);

    // Create and insert the new rule (TODO: Fix possible invalid rules and injection of extra rules)
    std::string addRule = fileglob + " " + filemode + ",\n";
    file_contents.insert(start_pos, addRule);

    // Push changes to 'output_file' and update changes
    output << file_contents;
    update_from_file_contents();
}
