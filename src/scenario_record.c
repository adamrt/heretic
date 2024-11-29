#include <assert.h>

#include "bin.h"
#include "event.h"
#include "scenario_record.h"

#define SCENARIO_FILE_OFFSET (0x10938)
#define SCENARIO_SIZE        (24)

scenario_record_t scenario_get_record(int scenario_id)
{

    file_t f = read_file_attack_out();
    f.offset = SCENARIO_FILE_OFFSET + (scenario_id * SCENARIO_SIZE);

    uint8_t bytes[SCENARIO_SIZE];
    read_bytes(&f, sizeof(bytes), bytes);

    scenario_record_t record = {
        .event_id = bytes[0] | (bytes[1] << 8),
        .map_id = bytes[2],
        .weather = bytes[3],
        .time = bytes[4],
        .entd_id = bytes[7] | (bytes[8] << 8),
        .next_scenario_id = bytes[18] | (bytes[19] << 8),
    };

    event_t event = event_get_event(record.event_id);
    if (!event.valid) {
        return record;
    }

    record.valid = true;

    return record;
}

// Thanks to FFTPAtcher for the scenario name list.
// https://github.com/Glain/FFTPatcher/blob/master/EntryEdit/EntryData/PSX/ScenarioNames.xml
scenario_name_t scenario_name_list[500] = {
    { 0x0000, "Unusable" },
    { 0x0001, "Orbonne Prayer (Setup)" },
    { 0x0002, "Orbonne Prayer" },
    { 0x0003, "Orbonne Battle (Setup)" },
    { 0x0004, "Orbonne Battle" },
    { 0x0005, "Orbonne Battle (Gafgarion and Agrias chat)" },
    { 0x0006, "Orbonne Battle (Abducting the Princess)" },
    { 0x0007, "Military Academy (Setup)" },
    { 0x0008, "Military Academy" },
    { 0x0009, "Gariland Fight (Setup)" },
    { 0x000A, "Gariland Fight" },
    { 0x000B, "Gariland Fight (Ramza, Delita, Thief Chat)" },
    { 0x000C, "Gariland Fight (Ramza talking about honest lives)" },
    { 0x000D, "Balbanes's Death (Setup)" },
    { 0x000E, "Balbanes's Death" },
    { 0x000F, "Mandalia Plains (Setup)" },
    { 0x0010, "Mandalia Plains (Options Given)" },
    { 0x0011, "Mandalia Plains (Destroy Corps Chosen)" },
    { 0x0012, "Mandalia Plains (Save Algus Chosen)" },
    { 0x0013, "Mandalia Plains (Algus First Turn)" },
    { 0x0014, "Mandalia Plains (Algus KO'd, Destroy Chosen)" },
    { 0x0015, "Mandalia Plains (Algus KO'd, Save Chosen)" },
    { 0x0016, "Mandalia Plains (Victory, Algus KO'd)" },
    { 0x0017, "Mandalia Plains (Victory, Algus Alive)" },
    { 0x0018, "Introducing Algus (Setup)" },
    { 0x0019, "Introducing Algus" },
    { 0x001A, "Returning to Igros (Setup)" },
    { 0x001B, "Returning to Igros" },
    { 0x001C, "Family Meeting (Setup)" },
    { 0x001D, "Family Meeting" },
    { 0x001E, "Sweegy Woods (Setup)" },
    { 0x001F, "Sweegy Woods" },
    { 0x0020, "Sweegy Woods (Victory)" },
    { 0x0021, "Dorter Trade City1 (Setup)" },
    { 0x0022, "Dorter Trade City1" },
    { 0x0023, "Dorter Trade City1 (Algus and Delita talk)" },
    { 0x0024, "Dorter Trade City1 (Victory)" },
    { 0x0025, "Interrogation (Setup)" },
    { 0x0026, "Interrogation" },
    { 0x0027, "Sand Rat Cellar (Setup)" },
    { 0x0028, "Sand Rat Cellar" },
    { 0x0029, "Sand Rat Cellar (Victory)" },
    { 0x002A, "Gustav vs. Wiegraf (Setup)" },
    { 0x002B, "Gustav vs. Wiegraf" },
    { 0x002C, "Larg's Praise (Setup)" },
    { 0x002D, "Larg's Praise" },
    { 0x002E, "Miluda1 (Setup)" },
    { 0x002F, "Miluda1" },
    { 0x0030, "Miluda1 (Miluda and Algus arguing)" },
    { 0x0031, "Miluda1 (Delita talking)" },
    { 0x0032, "Miluda1 (Victory)" },
    { 0x0033, "Releasing Miluda (Setup)" },
    { 0x0034, "Releasing Miluda" },
    { 0x0035, "Attack on the Beoulves (Setup)" },
    { 0x0036, "Attack on the Beoulves" },
    { 0x0037, "Meeting with bedridden Dycedarg (Setup)" },
    { 0x0038, "Meeting with bedridden Dycedarg" },
    { 0x0039, "Expelling Algus (Setup)" },
    { 0x003A, "Expelling Algus" },
    { 0x003B, "Reed Whistle (Setup)" },
    { 0x003C, "Reed Whistle" },
    { 0x003D, "Miluda2 (Setup)" },
    { 0x003E, "Miluda2" },
    { 0x003F, "Miluda2 (Delita talking with Miluda)" },
    { 0x0040, "Miluda2 (Miluda half HP)" },
    { 0x0041, "Miluda2 (Ramza debating with Miluda)" },
    { 0x0042, "Miluda2 (Ramza pleading with Miluda)" },
    { 0x0043, "Miluda2 (Miluda's Death)" },
    { 0x0044, "Wiegraf berating Golagros (Setup)" },
    { 0x0045, "Wiegraf berating Golagros" },
    { 0x0046, "Wiegraf1 (Setup)" },
    { 0x0047, "Wiegraf1" },
    { 0x0048, "Wiegraf1 (Delita, Ramza, Wiegraf talk)" },
    { 0x0049, "Wiegraf1 (Ramza and Wiegraf debate)" },
    { 0x004A, "Wiegraf1 (Ramza and Wiegraf talk)" },
    { 0x004B, "Wiegraf1 (Victory)" },
    { 0x004C, "Finding Teta Missing (Setup)" },
    { 0x004D, "Finding Teta Missing" },
    { 0x004E, "Fort Zeakden (Setup)" },
    { 0x004F, "Fort Zeakden" },
    { 0x0050, "Fort Zeakden (Algus, Ramza round 1)" },
    { 0x0051, "Fort Zeakden (Algus, Ramza round 2)" },
    { 0x0052, "Fort Zeakden (Algus, Ramza round 3)" },
    { 0x0053, "Fort Zeakden (Destroy Chosen at Mandalia)" },
    { 0x0054, "Fort Zeakden (Save Chosen at Mandalia)" },
    { 0x0055, "Fort Zeakden (Delita's First Turn)" },
    { 0x0056, "Fort Zeakden (Algus, Delita round 1)" },
    { 0x0057, "Fort Zeakden (Ramza, Delita talking)" },
    { 0x0058, "Fort Zeakden (Victory)" },
    { 0x0059, "Partings (Setup)" },
    { 0x005A, "Partings" },
    { 0x005B, "Deep Dungeon NOGIAS (Setup)" },
    { 0x005C, "Deep Dungeon NOGIAS (Battle)" },
    { 0x005D, "Deep Dungeon Panel Found" },
    { 0x005E, "Deep Dungeon (Victory - Used for all Floors)" },
    { 0x005F, "Deep Dungeon TERMINATE(Setup)" },
    { 0x0060, "Deep Dungeon TERMINATE (Battle)" },
    { 0x0061, "Deep Dungeon DELTA (Setup)" },
    { 0x0062, "Deep Dungeon DELTA (Battle)" },
    { 0x0063, "Deep Dungeon VALKYRIES (Setup)" },
    { 0x0064, "Deep Dungeon VALKYRIES (Battle)" },
    { 0x0065, "Deep Dungeon MLAPAN (Setup)" },
    { 0x0066, "Deep Dungeon MLAPAN (Battle)" },
    { 0x0067, "Deep Dungeon TIGER (Setup)" },
    { 0x0068, "Deep Dungeon TIGER (Battle)" },
    { 0x0069, "Deep Dungeon BRIDGE (Setup)" },
    { 0x006A, "Deep Dungeon BRIDGE (Battle)" },
    { 0x006B, "Deep Dungeon VOYAGE (Setup)" },
    { 0x006C, "Deep Dungeon VOYAGE (Battle)" },
    { 0x006D, "Deep Dungeon HORROR (Setup)" },
    { 0x006E, "Deep Dungeon HORROR (Battle)" },
    { 0x006F, "Elidibs (Setup)" },
    { 0x0070, "Elidibs" },
    { 0x0071, "Elidibs (Victory)" },
    { 0x0072, "Deep Dungeon END (Setup)" },
    { 0x0073, "Deep Dungeon END (Battle)" },
    { 0x0074, "Chapter 2 Start (Setup)" },
    { 0x0075, "Chapter 2 Start" },
    { 0x0076, "Dorter2 (Setup)" },
    { 0x0077, "Dorter2" },
    { 0x0078, "Dorter2 (Victory)" },
    { 0x0079, "Araguay Woods (Setup)" },
    { 0x007A, "Araguay Woods (Options Given)" },
    { 0x007B, "Araguay Woods (Kill Enemies Chosen)" },
    { 0x007C, "Araguay Woods (Save Boco Chosen)" },
    { 0x007D, "Araguay Woods (Boco KO'd, Kill Enemies Chosen)" },
    { 0x007E, "Araguay Woods (Boco KO'd, Save Boco Chosen)" },
    { 0x007F, "Araguay Woods (Victory)" },
    { 0x0080, "Zirekile Falls (Setup)" },
    { 0x0081, "Zirekile Falls" },
    { 0x0082, "Zirekile Falls (Gafgarion and Agrias talk)" },
    { 0x0083, "Zirekile Falls (Gafgarion, Ramza, Delita, talk)" },
    { 0x0084, "Zirekile Falls (Delita, Ovelia talk)" },
    { 0x0085, "Zirekile Falls (Ovelia's Death)" },
    { 0x0086, "Zirekile Falls (Gafgarion and Ramza arguing)" },
    { 0x0087, "Zirekile Falls (Gafgarion retreat)" },
    { 0x0088, "Zirekile Falls (Victory)" },
    { 0x0089, "Ovelia Joins (Setup)" },
    { 0x008A, "Ovelia Joins" },
    { 0x008B, "Zalamd Fort City (Setup)" },
    { 0x008C, "Zaland Fort City (Options Given)" },
    { 0x008D, "Zaland Fort City (Kill Enemies Chosen)" },
    { 0x008E, "Zaland Fort City (Save Mustadio Chosen)" },
    { 0x008F, "Zaland Fort City (Mustadio KO'd, Kill Chosen)" },
    { 0x0090, "Zaland Fort City (Mustadio KO'd, Save Chosen)" },
    { 0x0091, "Zaland Fort City (Victory)" },
    { 0x0092, "Ramza, Mustadio, Agrias and Ovelia meeting (Setup)" },
    { 0x0093, "Ramza, Mustadio, Agrias and Ovelia meeting" },
    { 0x0094, "Ruins of Zaland (Setup)" },
    { 0x0095, "Ruins of Zaland" },
    { 0x0096, "Bariaus Hill (Setup)" },
    { 0x0097, "Bariaus Hill" },
    { 0x0098, "Bariaus Hill (Victory)" },
    { 0x0099, "Dycedarg and Gafgarion Reunion (Setup)" },
    { 0x009A, "Dycedarg and Gafgarion Reunion" },
    { 0x009B, "Gate of Lionel Castle (Setup)" },
    { 0x009C, "Gate of Lionel Castle" },
    { 0x009D, "Meeting with Draclay (Setup)" },
    { 0x009E, "Meeting with Draclau" },
    { 0x009F, "Besrodio Kidnapped (Setup)" },
    { 0x00A0, "Besrodio Kidnapped" },
    { 0x00A1, "Zigolis Swamp (Setup)" },
    { 0x00A2, "Zigolis Swamp" },
    { 0x00A3, "Zigolis Swamp (Victory)" },
    { 0x00A4, "Goug Machine City Town (Setup)" },
    { 0x00A5, "Goug Machine City Town" },
    { 0x00A6, "Goug Machine City (Setup)" },
    { 0x00A7, "Goug Machine City" },
    { 0x00A8, "Goug Machine City (Victory)" },
    { 0x00A9, "Besrodio Saved (Setup)" },
    { 0x00AA, "Besrodio Saved" },
    { 0x00AB, "Warjilis Port (Setup)" },
    { 0x00AC, "Warjilis Port" },
    { 0x00AD, "Draclau hires Gafgarion (Setup)" },
    { 0x00AE, "Draclau hires Gafgarion" },
    { 0x00AF, "Bariaus Valley (Setup)" },
    { 0x00B0, "Bariaus Valley" },
    { 0x00B1, "Bariaus Valley (Agrias and Ramza talk)" },
    { 0x00B2, "Bariaus Valley (Agrias Death)" },
    { 0x00B3, "Bariaus Valley (Victory)" },
    { 0x00B4, "Golgorand Execution Site (Setup)" },
    { 0x00B5, "Golgorand Execution Site" },
    { 0x00B6, "Golgorand Execution Site (Gafgarion and Agrias talk)" },
    { 0x00B7, "Golgorand Execution Site (Gafgarion and Ramza talk first part)" },
    { 0x00B8, "Golgorand Execution Site (Gafgarion and Ramza talk second part)" },
    { 0x00B9, "Golgorand Execution Site (Gafgarion and Ramza talk third part)" },
    { 0x00BA, "Golgorand Execution Site (Gafgarion, Agrias and Ramza talk)" },
    { 0x00BB, "Golgorand Execution Site (Gafgarion retreats)" },
    { 0x00BC, "Golgorand Execution Site (Victory)" },
    { 0x00BD, "Substitute (Setup)" },
    { 0x00BE, "Substitute" },
    { 0x00BF, "Lionel Castle Gate (Setup)" },
    { 0x00C0, "Lionel Castle Gate" },
    { 0x00C1, "Lionel Castle Gate (Ramza opens the gate)" },
    { 0x00C2, "Lionel Castle Gate (Gafgarion Death)" },
    { 0x00C3, "Lionel Castle Gate (Victory)" },
    { 0x00C4, "Inside of Lionel Castle (Setup)" },
    { 0x00C5, "Inside of Lionel Castle" },
    { 0x00C6, "Inside of Lionel Castle (Queklain and Ramza talk)" },
    { 0x00C7, "Inside of Lionel Castle (Victory)" },
    { 0x00C8, "The Lion War Outbreak (Setup)" },
    { 0x00C9, "The Lion War Outbreak" },
    { 0x00CA, "Chapter 3 Start (Setup)" },
    { 0x00CB, "Chapter 3 Start" },
    { 0x00CC, "Goland Coal City (Setup)" },
    { 0x00CD, "Goland Coal City" },
    { 0x00CE, "Goland Coal City (Olan Death)" },
    { 0x00CF, "Goland Coal City (Victory)" },
    { 0x00D0, "Goland Coal City post battle (setup)" },
    { 0x00D1, "Goland Coal City post battle" },
    { 0x00D2, "Steel Ball Found! (Setup)" },
    { 0x00D3, "Steel Ball Found!" },
    { 0x00D4, "Worker 8 Activated (setup)" },
    { 0x00D5, "Worker 8 Activated" },
    { 0x00D6, "Summoning Machine Found! (Setup)" },
    { 0x00D7, "Summoning Machine Found!" },
    { 0x00D8, "Cloud Summoned (Setup)" },
    { 0x00D9, "Cloud Summoned" },
    { 0x00DA, "Zarghidas (Setup)" },
    { 0x00DB, "Zarghidas" },
    { 0x00DC, "Zarghidas (Cloud freaking out)" },
    { 0x00DD, "Zarghidas (Cloud Death)" },
    { 0x00DE, "Zarghidas (Victory)" },
    { 0x00DF, "Talk with Zalbag in Lesalia (Setup)" },
    { 0x00E0, "Talk with Zalbag in Lesalia" },
    { 0x00E1, "Outside Castle Gate in Lesalia Zalmo 1 (Setup)" },
    { 0x00E2, "Outside Castle Gate in Lesalia Zalmo 1" },
    { 0x00E3, "Outside Castle Gate in Lesalia Zalmo 1 (Zalmo and Ramza talk)" },
    { 0x00E4, "Outside Castle Gate in Lesalia Zalmo 1 (Alma and Ramza talk)" },
    { 0x00E5, "Outside Castle Gate in Lesalia Zalmo 1 (Victory)" },
    { 0x00E6, "Outside Castle Gate in Lesalia Talk with Alma (Setup)" },
    { 0x00E7, "Outside Castle Gate in Lesalia Talk with Alma" },
    { 0x00E8, "Orbonne Monastery (Setup)" },
    { 0x00E9, "Orbonne Monastery" },
    { 0x00EA, "Underground Book Storage Second Floor (Setup)" },
    { 0x00EB, "Underground Book Storage Second Floor" },
    { 0x00EC, "Underground Book Storage Second Floor (Victory)" },
    { 0x00ED, "Underground Book Storage Third Floor (Setup)" },
    { 0x00EE, "Underground Book Storage Third Floor" },
    { 0x00EF, "Underground Book Storage Third Floor (Izlude, Ramza talk first)" },
    { 0x00F0, "Underground Book Storage Third Floor (Izlude, Ramza talk second)" },
    { 0x00F1, "Underground Book Storage Third Floor (Victory)" },
    { 0x00F2, "Underground Book Storage First Floor (Setup)" },
    { 0x00F3, "Underground Book Storage First Floor" },
    { 0x00F4, "Underground Book Storage First Floor (Wiegraf talk)" },
    { 0x00F5, "Underground Book Storage First Floor (Wiegraf, Ramza talk first)" },
    { 0x00F6, "Underground Book Storage First Floor (Wiegraf, Ramza talk second)" },
    { 0x00F7, "Underground Book Storage First Floor (Victory)" },
    { 0x00F8, "Meet Velius (Setup)" },
    { 0x00F9, "Meet Velius" },
    { 0x00FA, "Malak and the Scriptures (Setup)" },
    { 0x00FB, "Malak and the Scriptures (Options Given)" },
    { 0x00FC, "Malak and the Scriptures (Yes Chosen)" },
    { 0x00FD, "Malak and the Scriptures (No Chosen)" },
    { 0x00FE, "Delita swears allegiance to Ovelia (Setup)" },
    { 0x00FF, "Delita swears allegiance to Ovelia" },
    { 0x0100, "Grog Hill (Setup)" },
    { 0x0101, "Grog Hill" },
    { 0x0102, "Grog Hill (Victory)" },
    { 0x0103, "Meet Again with Olan (Setup)" },
    { 0x0104, "Meet again with Olan" },
    { 0x0105, "Rescue Rafa (Setup)" },
    { 0x0106, "Rescue Rafa" },
    { 0x0107, "Rescue Rafa (Malak and Ramza talk)" },
    { 0x0108, "Rescue Rafa (Malak, Ninja and Ramza talk)" },
    { 0x0109, "Rescue Rafa (Malak Retreat)" },
    { 0x010A, "Rescue Rafa (Rafa Death, Malak Present)" },
    { 0x010B, "Rescue Rafa (Rafa Death, Malak Retreated)" },
    { 0x010C, "Rescue Rafa (Victory)" },
    { 0x010D, "Exploding Frog (Setup)" },
    { 0x010E, "Exploding Frog" },
    { 0x010F, "Yuguo Woods (Setup)" },
    { 0x0110, "Yuguo Woods" },
    { 0x0111, "Yuguo Woods (Victory)" },
    { 0x0112, "Barinten threatens Vormav (Setup)" },
    { 0x0113, "Barinten threatens Vormav" },
    { 0x0114, "Riovanes Castle Entrance (Setup)" },
    { 0x0115, "Riovanes Castle Entrance" },
    { 0x0116, "Riovanes Castle Entrance (Rafa, Malak and Ramza talk)" },
    { 0x0117, "Riovanes Castle Entrance (Malak Defeated)" },
    { 0x0118, "Riovanes Castle Entrance (Rafa Defeated)" },
    { 0x0119, "Riovanes Castle Entrance (Victory)" },
    { 0x011A, "Escaping Alma (Setup)" },
    { 0x011B, "Escaping Alma" },
    { 0x011C, "Inside of Riovanes Castle (Setup)" },
    { 0x011D, "Inside of Riovanes Castle" },
    { 0x011E, "Inside of Riovanes Castle (Wiegraf and Ramza talk)" },
    { 0x011F, "Inside of Riovanes Castle (Here comes Velius)" },
    { 0x0120, "Inside of Riovanes Castle (Victory)" },
    { 0x0121, "Ajora's vessel (Setup)" },
    { 0x0122, "Ajora's vessel" },
    { 0x0123, "Rooftop of Riovanes Castle (Setup)" },
    { 0x0124, "Rooftop of Riovanes Castle" },
    { 0x0125, "Rooftop of Riovanes Castle (Rafa Death)" },
    { 0x0126, "Rooftop of Riovanes Castle (Victory)" },
    { 0x0127, "Reviving Malak (Setup)" },
    { 0x0128, "Reviving Malak" },
    { 0x0129, "Searching for Alma (Setup)" },
    { 0x012A, "Searching for Alma" },
    { 0x012B, "Things Obtained (Setup)" },
    { 0x012C, "Things Obtained" },
    { 0x012D, "Underground Book Storage Fourth Floor (Setup)" },
    { 0x012E, "Underground Book Storage Fourth Floor" },
    { 0x012F, "Underground Book Storage Fourth Floor (Victory)" },
    { 0x0130, "Underground Book Storage Fifth Floor (Setup)" },
    { 0x0131, "Underground Book Storage Fifth Floor" },
    { 0x0132, "Underground Book Storage Fifth Floor (Rofel and Ramza talk)" },
    { 0x0133, "Underground Book Storage Fifth Floor (Victory)" },
    { 0x0134, "Entrance to the other world (Setup)" },
    { 0x0135, "Entrance to the other world" },
    { 0x0136, "Murond Death City (Setup)" },
    { 0x0137, "Murond Death City" },
    { 0x0138, "Murond Death City (Kletian and Ramza talk)" },
    { 0x0139, "Murond Death City (Victory)" },
    { 0x013A, "Lost Sacred Precincts (Setup)" },
    { 0x013B, "Lost Sacred Precincts" },
    { 0x013C, "Lost Sacred Precincts (Balk and Ramza talk)" },
    { 0x013D, "Lost Sacred Precincts (Victory)" },
    { 0x013E, "Graveyard of Airships (Setup)" },
    { 0x013F, "Graveyard of Airships" },
    { 0x0140, "Graveyard of Airships (Hashmalum and Ramza talk)" },
    { 0x0141, "Graveyard of Airships (Victory)" },
    { 0x0142, "Graveyard of Airships (Setup)" },
    { 0x0143, "Graveyard of Airships" },
    { 0x0144, "Graveyard of Airships (Here comes Altima 2)" },
    { 0x0145, "Graveyard of Airships (Victory)" },
    { 0x0146, "Reunion and Beyond" },
    { 0x0147, "Reunion and beyond" },
    { 0x0148, "Those Who Squirm In Darkness (Setup)" },
    { 0x0149, "Those Who Squirm in Darkness" },
    { 0x014A, "A Man with the Holy Stone (Setup)" },
    { 0x014B, "A Man with the Holy Stone" },
    { 0x014C, "Doguola Pass (Setup)" },
    { 0x014D, "Doguola Pass" },
    { 0x014E, "Doguola Pass (Victory)" },
    { 0x014F, "Bervenia Free City (Setup)" },
    { 0x0150, "Bervenia Free City" },
    { 0x0151, "Bervenia Free City (Meliadoul and Ramza talk first part)" },
    { 0x0152, "Bervenia Free City (Meliadoul and Ramza talk second part)" },
    { 0x0153, "Bervenia Free City (Meliadoul and Ramza talk third part)" },
    { 0x0154, "Bervenia Free City (Victory)" },
    { 0x0155, "Finath River (Setup)" },
    { 0x0156, "Finath River" },
    { 0x0157, "Finath River (Victory)" },
    { 0x0158, "Delita's Thoughts (Setup)" },
    { 0x0159, "Delita's Thoughts" },
    { 0x015A, "Zalmo II (Setup)" },
    { 0x015B, "Zalmo II" },
    { 0x015C, "Zalmo II (Zalmo and Delita talk)" },
    { 0x015D, "Zalmo II (Zalmo and Ramza talk)" },
    { 0x015E, "Zalmo II (Victory)" },
    { 0x015F, "Unstoppable Cog (Setup)" },
    { 0x0160, "Unstoppable Cog" },
    { 0x0161, "Balk I (Setup)" },
    { 0x0162, "Balk I" },
    { 0x0163, "Balk I (Balk and Ramza talk)" },
    { 0x0164, "Balk I (Victory)" },
    { 0x0165, "Seized T" },
    { 0x0166, "Seized T" },
    { 0x0167, "South Wall of Bethla Garrison (Setup)" },
    { 0x0168, "South Wall of Bethla Garrison" },
    { 0x0169, "South Wall of Bethla Garrison (Victory)" },
    { 0x016A, "North Wall of Bethla Garrison (Setup)" },
    { 0x016B, "North Wall of Bethla Garrison" },
    { 0x016C, "North Wall of Bethla Garrison (Victory)" },
    { 0x016D, "Assassination of Prince Larg (Setup)" },
    { 0x016E, "Assassination of Prince Larg" },
    { 0x016F, "Bethla Sluice (Setup)" },
    { 0x0170, "Bethla Sluice" },
    { 0x0171, "Bethla Sluice (First lever left)" },
    { 0x0172, "Bethla Sluice (Second lever left)" },
    { 0x0173, "Bethla Sluice (First lever right)" },
    { 0x0174, "Bethla Sluice (Second lever right)" },
    { 0x0175, "Rescue of Cid (Setup)" },
    { 0x0176, "Rescue of Cid" },
    { 0x0177, "Prince Goltana's Final Moments (Setup)" },
    { 0x0178, "Prince Goltana's Final Moments" },
    { 0x0179, "Germinas Peak (Setup)" },
    { 0x017A, "Germinas Peak" },
    { 0x017B, "Germinas Peak (Victory)" },
    { 0x017C, "Poeskas Lake (Setup)" },
    { 0x017D, "Poeskas Lake" },
    { 0x017E, "Poeskas Lake (Victory)" },
    { 0x017F, "Ambition of Dycedarg (Setup)" },
    { 0x0180, "Ambition of Dycedarg" },
    { 0x0181, "Outside of Limberry Castle (Setup)" },
    { 0x0182, "Outside of Limberry Castle" },
    { 0x0183, "Outside of Limberry Castle (Victory)" },
    { 0x0184, "Men of Odd Appearance (Setup)" },
    { 0x0185, "Men of Odd Appearance" },
    { 0x0186, "Elmdor II (Setup)" },
    { 0x0187, "Elmdor II" },
    { 0x0188, "Elmdor II (Ultima Demon Celia)" },
    { 0x0189, "Elmdor II (Ultima Demon Lede)" },
    { 0x018A, "Elmdor II (Victory)" },
    { 0x018B, "Zalera (Setup)" },
    { 0x018C, "Zalera" },
    { 0x018D, "Zalera (Zalera, Meliadoul and Ramza talk)" },
    { 0x018E, "Zalera (Meliadoul and Ramza talk)" },
    { 0x018F, "Zalera (Victory)" },
    { 0x0190, "Random Battle Template (Setup)" },
    { 0x0191, "Random Battle Template (Initiate)" },
    { 0x0192, "Random Battle Template (Victory)" },
    { 0x0193, "Empty" },
    { 0x0194, "Game Over Event (Plays automatically upon Game Over)" },
    { 0x0195, "Empty" },
    { 0x0196, "Empty" },
    { 0x0197, "Empty" },
    { 0x0198, "Empty" },
    { 0x0199, "Empty" },
    { 0x019A, "Tutorial - (Battlefield Control) (Setup)" },
    { 0x019B, "Tutorial - (Battlefield Control)" },
    { 0x019C, "Tutorial - (Battle) (Setup)" },
    { 0x019D, "Tutorial - (Battle)" },
    { 0x019E, "Tutorial - (Move and Act) (Setup)" },
    { 0x019F, "Tutorial - (Move and Act)" },
    { 0x01A0, "Tutorial - (Charge Time Battle) (Setup)" },
    { 0x01A1, "Tutorial - (Charge Time Battle)" },
    { 0x01A2, "Tutorial - (How to Cast Spells) (Setup)" },
    { 0x01A3, "Tutorial - (How to Cast Spells)" },
    { 0x01A4, "Tutorial - (Abnormal Status) (Setup)" },
    { 0x01A5, "Tutorial - (Abnormal Status)" },
    { 0x01A6, "Tutorial - (On-Line Help) (Setup)" },
    { 0x01A7, "Tutorial - (On-Line Help)" },
    { 0x01A8, "Tutorial - (Options) (Setup)" },
    { 0x01A9, "Tutorial - (Options)" },
    { 0x01AA, "The Mystery of Lucavi (Setup)" },
    { 0x01AB, "The Mystery of Lucavi" },
    { 0x01AC, "Delita's Betrayal (Setup)" },
    { 0x01AD, "Delita's Betrayal" },
    { 0x01AE, "Delita's Betrayal" },
    { 0x01AF, "Mosfungus (Setup)" },
    { 0x01B0, "Mosfungus" },
    { 0x01B1, "At the Gate of the Beoulve Castle (Setup)" },
    { 0x01B2, "At the Gate of the Beoulve Castle" },
    { 0x01B3, "Adramelk (Setup)" },
    { 0x01B4, "Adramelk" },
    { 0x01B5, "Adramelk (Zalbag and Ramza talk)" },
    { 0x01B6, "Adramelk (Dycedarg and Zalbag talk)" },
    { 0x01B7, "Adramelk (Here comes Adramelk)" },
    { 0x01B8, "Adramelk (Victory)" },
    { 0x01B9, "Funeral's Final Moments (Setup)" },
    { 0x01BA, "Funeral's Final Moments" },
    { 0x01BB, "St. Murond Temple (Setup)" },
    { 0x01BC, "St. Murond Temple" },
    { 0x01BD, "St. Murond Temple (Victory)" },
    { 0x01BE, "Hall of St. Murond Temple (Setup)" },
    { 0x01BF, "Hall of St. Murond Temple" },
    { 0x01C0, "Hall of St. Murond Temple (Vormav and Meliadoul talk)" },
    { 0x01C1, "Hall of St. Murond Temple (Vormav and Ramza talk)" },
    { 0x01C2, "Hall of St. Murond Temple (Victory)" },
    { 0x01C3, "Chapel of St. Murond Temple (Setup)" },
    { 0x01C4, "Chapel of St. Murond Temple" },
    { 0x01C5, "Chapel of St. Murond Temple (Zalbag, Ramza first turn)" },
    { 0x01C6, "Chapel of St. Murond Temple (Ramza, Zalbag 50% HP talk)" },
    { 0x01C7, "Chapel of St. Murond Temple (Victory)" },
    { 0x01C8, "Requiem (Setup)" },
    { 0x01C9, "Requiem" },
    { 0x01CA, "Zarghidas (Setup)" },
    { 0x01CB, "Zarghidas (Options Given)" },
    { 0x01CC, "Zarghidas (Don't Buy Flower Chosen)" },
    { 0x01CD, "Zarghidas (Buy Flower Chosen)" },
    { 0x01CE, "Bar - Deep Dungeon (Setup)" },
    { 0x01CF, "Bar - Deep Dungeon" },
    { 0x01D0, "Bar - Goland Coal City (Setup)" },
    { 0x01D1, "Bar - Goland Coal City (Options Given)" },
    { 0x01D2, "Bar - Goland Coal City (Refuse Beowulf's Invitation Chosen)" },
    { 0x01D3, "Bar - Goland Coal City (Accept Beowulf's invitation Chosen)" },
    { 0x01D4, "Colliery Underground - Third Floor (Setup)" },
    { 0x01D5, "Colliery Underground - Third Floor (Battle)" },
    { 0x01D6, "Colliery Underground - Third Floor (Victory)" },
    { 0x01D7, "Colliery Underground - Second Floor (Setup)" },
    { 0x01D8, "Colliery Underground - Second Floor (Battle)" },
    { 0x01D9, "Colliery Underground - Second Floor (Victory)" },
    { 0x01DA, "Colliery Underground - First Floor (Setup)" },
    { 0x01DB, "Colliery Underground - First Floor (Battle)" },
    { 0x01DC, "Colliery Underground - First Floor (Victory)" },
    { 0x01DD, "Underground Passage in Goland (Setup)" },
    { 0x01DE, "Underground Passage in Goland (Battle)" },
    { 0x01DF, "Underground Passage in Goland (Reis's Death, Beowulf Alive)" },
    { 0x01E0, "Underground Passage in Goland (Reis's Death, Beowulf KO'd)" },
    { 0x01E1, "Underground Passage in Goland (Victory)" },
    { 0x01E2, "Underground Passage in Goland (Setup)" },
    { 0x01E3, "Underground Passage in Goland (Post-Battle)" },
    { 0x01E4, "Nelveska Temple (Setup)" },
    { 0x01E5, "Nelveska Temple" },
    { 0x01E6, "Nelveska Temple (Worker 7 recharging)" },
    { 0x01E7, "Nelveska Temple (Victory)" },
    { 0x01E8, "Reis Curse (Setup)" },
    { 0x01E9, "Reis Curse" },
    { 0x01EA, "Bethla Sluice (Late add-in Ramza hint)" },
    { 0x01EB, "Empty" },
    { 0x01EC, "Empty" },
    { 0x01ED, "Empty" },
    { 0x01EE, "Empty" },
    { 0x01EF, "Empty" },
    { 0x01F0, "Empty" },
    { 0x01F1, "Empty" },
    { 0x01F2, "Empty" },
    { 0x01F3, "Empty" },
};