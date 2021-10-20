/*!
 * @file Configurator_test.cpp
 *
 * @date Oct 8, 2021
 * @author Tim Spain <timothy.spain@nersc.no>
 */

#include "Configurator.hpp"
#include "Configured.hpp"
#include "ArgV.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <memory>

#include <boost/program_options.hpp>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

class Config1 {
public:
    Config1()
        : value(0)
    {
    }
    void configure()
    {
        const std::string valueKey = "config.value";

        boost::program_options::options_description opt("Options");
        opt.add_options()
                (valueKey.c_str(), boost::program_options::value<int>()->default_value(-1), "Specify a value")
                ;
        boost::program_options::variables_map vm = Nextsim::Configurator::parse(opt);

        value = vm[valueKey].as<int>();
    }

    bool checkValue(int target)
    {
        return value == target;
    }
private:
    int value;
};

class Config2 : public Nextsim::Configured {
public:
    Config2()
        : value(0)
    {
        addOption<int>(valueKey, -1, "Specify a value");
        addOption<std::string>(nameKey, "", "Specify a name");
    }
    int getValue()
    {
        return value;
    }
    std::string getName()
    {
        return name;
    }
protected:
    virtual void parse()
    {
        Configured::parse();

        value = retrieveValue<int>(valueKey);
        name = retrieveValue<std::string>(nameKey);
    }

private:
    int value;
    std::string name;
    const std::string valueKey = "config.value";
    const std::string nameKey = "config.name";

};

class Config3 : public Nextsim::Configured {
public:
    Config3()
        : value(0)
        , weight(0.)
    {
        addOption<int>(valueKey, -1, "Specify a value");
        addOption<double>(weightKey, 1., "Specify a weight");
    }
    int getValue()
    {
        return value;
    }
    double getWeight()
    {
        return weight;
    }
protected:
    virtual void parse()
    {
        Configured::parse();

        value = retrieveValue<int>(valueKey);
        weight = retrieveValue<double>(weightKey);
    }
private:
    int value;
    double weight;
    const std::string valueKey = "config.value";
    const std::string weightKey = "data.weight";

};

namespace Nextsim {

TEST_CASE("Parse one config stream using the raw configurator", "[Configurator]")
{
    // Since tests are not different execution environments, clear the streams from other tests.
    Configurator::clearStreams();
    Config1 config;

    int target = 42;
    std::stringstream text;
    text << "[config]" << std::endl
            << "value = " << target << std::endl;

    // Check for the initialized value
    REQUIRE(config.checkValue(0));

    config.configure();
    // Check for the default initialized value
    REQUIRE(config.checkValue(-1));

    std::unique_ptr<std::istream> pcstream(new std::stringstream(text.str()));

    Configurator::addStream(std::move(pcstream));

    config.configure();

    // Check for the configured value
    REQUIRE(config.checkValue(target));
}

TEST_CASE("Parse one config stream using the auto configurator", "[Configurator]")
{
    // Since tests are not different execution environments, clear the streams from other tests.
    Configurator::clearStreams();
    Config2 config;

    int target = 69105;
    std::string targetName = "Zork";
    std::stringstream text;
    text << "[config]" << std::endl
            << "value = " << target << std::endl
            << "name = " << targetName << std::endl;

    Configurator::addStream(std::unique_ptr<std::istream>(new std::stringstream(text.str())));

    Configured::configureAll();

    REQUIRE(config.getValue() == target);
    REQUIRE(config.getName() == targetName);

}

TEST_CASE("Parse two config streams for one class, auto", "[Configurator]")
{
    // Since tests are not different execution environments, clear the streams from other tests.
    Configurator::clearStreams();
    Config2 config;

    int target = 69105;
    std::string targetName = "Zork";
    std::stringstream text1;
    text1 << "[config]" << std::endl
            << "value = " << target << std::endl;
    Configurator::addStream(std::unique_ptr<std::istream>(new std::stringstream(text1.str())));

    std::stringstream text2;
    text2 << "[config]" << std::endl
            << "name = " << targetName << std::endl;
    Configurator::addStream(std::unique_ptr<std::istream>(new std::stringstream(text2.str())));

    Configured::configureAll();

    REQUIRE(config.getValue() == target);
    REQUIRE(config.getName() == targetName);
}

TEST_CASE("Parse config streams for two overlapping class, auto", "[Configurator]")
{
    // Since tests are not different execution environments, clear the streams from other tests.
    Configurator::clearStreams();
    Config2 config;
    Config3 confih;

    int target = 69105;
    std::string targetName = "Zork II";
    std::stringstream text1;
    text1 << "[config]" << std::endl
            << "value = " << target << std::endl
            << "name = " << targetName << std::endl;
    Configurator::addStream(std::unique_ptr<std::istream>(new std::stringstream(text1.str())));

    double targetWeight = 0.467836;
    std::stringstream text2;
    text2 << "[data]" << std::endl
            << "weight = " << targetWeight << std::endl;
    Configurator::addStream(std::unique_ptr<std::istream>(new std::stringstream(text2.str())));

    // Test that the target values are not already present before configuring
    REQUIRE(config.getValue() != target);
    REQUIRE(config.getName() != targetName);

    REQUIRE(confih.getValue() != target);
    REQUIRE(confih.getWeight() != targetWeight);

    Configured::configureAll();

    REQUIRE(config.getValue() == target);
    REQUIRE(config.getName() == targetName);

    REQUIRE(confih.getValue() == target);
    REQUIRE(confih.getWeight() == targetWeight);
}

} /* namespace Nextsim */