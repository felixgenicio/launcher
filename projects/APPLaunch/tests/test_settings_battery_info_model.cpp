/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "../main/ui/settings/settings_battery_info_model.hpp"

#include <array>
#include <cassert>
#include <string>

int main()
{
    SettingsBatteryInfoModel model;
    assert(!model.valid());
    assert(model.labels()[0] == "Battery: --%");
    assert(model.values()[0] == "--%");
    assert(model.values()[4] == "-- mAh");
    assert(model.values()[5] == "-- mAh");

    const std::string valid_payload = "3800,-250,250,78,1200,2000,1,-100,1,-1,76,2";
    assert(model.update(0, valid_payload));
    const auto previous_snapshot = model.snapshot();
    const auto previous_labels = model.labels();
    const auto previous_values = model.values();
    assert(previous_values[0] == "78%");
    assert(previous_values[1] == "25.0 C");
    assert(previous_values[2] == "-250 mA");
    assert(previous_values[3] == "3.80 V");
    assert(previous_values[4] == "1200 mAh");
    assert(previous_values[5] == "2000 mAh");
    assert(previous_values[6] == "1h16m");
    assert(previous_values[7] == "2");
    assert(model.charge_state_text() == "Charging - full in 1h16m");

    {
        SettingsBatteryInfoModel m;
        // Discharging: time to empty shown, stray time to full ignored.
        assert(m.update(0, "3700,-300,250,49,860,1750,8,-300,1,95,40,3"));
        assert(m.values()[6] == "1h35m" && m.labels()[6] == "Time to empty: 1h35m");
        assert(m.charge_state_text() == "Discharging - empty in 1h35m");
        // Full at 100 %: no time, even if the gauge reports one.
        assert(m.update(0, "4200,0,250,100,1750,1750,2,0,1,0,0,3"));
        assert(m.values()[6] == "--" && m.charge_state_text() == "Full");
        // Plugged in but not charging is not "Discharging".
        assert(m.update(0, "4180,0,250,98,1700,1750,4,0,1,500,-1,3"));
        assert(m.values()[6] == "--");
        assert(m.charge_state_text() == "Plugged in - not charging");
        // Charging with a 0 time to full: no "full in 0 min".
        assert(m.update(0, "4100,300,250,99,1740,1750,1,300,1,-1,0,3"));
        assert(m.values()[6] == "--" && m.charge_state_text() == "Charging");
        // Discharging with unknown time, and unknown state.
        assert(m.update(0, "3700,-300,250,49,860,1750,8,-300,1,-1,-1,-1"));
        assert(m.charge_state_text() == "Discharging" && m.values()[7] == "--");
        assert(m.update(0, "3700,-300,250,49,860,1750,0,-300,1,95,-1,3"));
        assert(m.values()[6] == "--" && m.charge_state_text() == "Unknown");
        // No capacity from the gauge.
        assert(m.update(0, "3700,-300,250,49,0,0,8,-300,1,-1,-1,-1"));
        assert(m.values()[4] == "-- mAh" && m.values()[5] == "-- mAh");
    }

    assert(!model.update(-1, "battery api failed"));
    assert(model.valid());
    assert(model.snapshot().voltage_mv == previous_snapshot.voltage_mv);
    assert(model.labels() == previous_labels);
    assert(model.values() == previous_values);
    assert(model.status_text() == "Battery read failed");

    assert(!model.update(0, "invalid payload"));
    assert(model.valid());

    // 12-field payloads from the current backend are accepted as well.
    {
        SettingsBatteryInfoModel extended;
        assert(extended.update(0, "3800,-250,250,78,1200,2000,1,-100,1,-1,76,2"));
        assert(extended.values()[0] == "78%");
        assert(extended.snapshot().voltage_mv == 3800);
        assert(!extended.update(0, "3800,-250,250,78,1200,2000,1,-100,1,-2,76,2"));
        assert(!extended.update(0, "3800,-250,250,78,1200,2000,1,-100,1,-1,76"));
        assert(!extended.update(0, "3800,-250,250,78,1200,2000,1,-100,1,-1,76,2,0"));
        assert(!extended.update(0, "3800,-250,250,78,1200,2000,1,-100,1,"));
    }
    assert(model.valid());
    assert(model.labels() == previous_labels);
    assert(model.values() == previous_values);
    assert(model.status_text() == "Invalid battery data");

    model.set_status("Reading battery...");
    assert(model.valid());
    assert(model.labels() == previous_labels);
    assert(model.values() == previous_values);

    model.invalidate();
    assert(!model.valid());
    assert(model.labels()[0] == "Battery: --%");
    assert(model.values()[0] == "--%");
    assert(model.values()[4] == "-- mAh");
    assert(model.values()[5] == "-- mAh");
}
