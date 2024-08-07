#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "FFT.h"

// Scenario represents a battle information.
class Scenario {
public:
    Scenario() = default;
    Scenario(std::vector<uint8_t> data)
        : data(data) {};

    bool operator==(const Scenario& other) const;

    auto repr() const -> std::string;

    // This is the ID of the scenario, but it is also the index into events.
    auto id() const -> int;

    // map_id() returns the ID from our map_list.
    auto map_id() const -> int;

    auto weather() const -> MapWeather;
    auto time() const -> MapTime;

    auto first_music() const -> int;
    auto second_music() const -> int;

    auto entd_id() const -> int;

    auto first_grid() const -> int;
    auto second_grid() const -> int;

    auto require_ramza_unknown() const -> int;

    // next_step() returns the next step in the scenario. Options:
    // - 0x80 = World map
    // - 0x81 = Scenario (next_scenario())
    // - 0x82 = Game reset follows
    auto next_step() const -> int;

    // next_scenario() returns the next scenario id if next_step() is 0x81.
    auto next_scenario() const -> int;

    // The docs describe this as the event_id() but you should use the
    // Scenario.id() as the index into events. This is currently unused. Actual
    // events that we care about (non setup) will have this as 0.
    auto event_id() const -> int;

private:
    std::vector<uint8_t> data;
};

extern std::map<int, std::string> scenario_list;
