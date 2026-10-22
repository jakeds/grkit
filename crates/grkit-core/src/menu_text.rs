//! The camera menu text in each menu language, and the label table that the
//! installer copies after the picture scratch (`runtime/abi.h`).
//!
//! The menu shows the language of the camera if it is a menu language, and
//! English otherwise. Effect names stay as their definitions give them.

use anyhow::{ensure, Result};

use crate::packaging::Language;

/// The size of the text area after the picture scratch.
pub const TEXT_SIZE: usize = 19456;
/// Where the Image Control summaries start in the text area. The label table
/// must end before it.
pub const TEXT_SUMMARY: usize = 13312;
/// The last 16 bytes of the text area: the parameter page title of a GRKit
/// session. Words: the page, its vtable, then the descriptor of the effect name.
pub const TEXT_PARAMETER_TITLE: usize = TEXT_SIZE - 16;
/// Two transient 32-unit buffers for the menu path and value.
pub const TEXT_MENU_PATH: usize = TEXT_SIZE - 144;
/// The units of the longest string with its NUL, and of each summary slot.
pub const SUMMARY_UNITS: usize = 32;
/// The number of strings in each language (`GRKIT_TEXT_STRINGS`).
pub const STRINGS: usize = {
    let mut n = 0;
    let mut i = 0;
    while i < RUNS.len() {
        n += RUNS[i].count;
        i += 1;
    }
    n
};
/// The longest effect name in a record.
const NAME_UNITS: usize = 16;

/// The words of one menu language. Each list follows the rows of its module
/// after Off. The Chinese and Japanese words reuse the terms of the camera
/// menus where they have them (影像控制, 关闭/關閉, 弱/中/强, 颗粒感/粒紋感;
/// イメージコントロール, オフ, 粒状感, グリッド, 9分割, 画面中央, 弱/中/強).
/// 加印日期 and 日付入れ come from the date imprint of earlier GR models
/// (日付入れ撮影 in the Japanese GR DIGITAL III manual).
/// The camera's Chinese and Japanese fonts draw each of these characters only
/// while the camera language is that language, which is when the menu shows
/// them.
struct Words {
    /// Between a module name and its value in a summary.
    separator: &'static str,
    image_control: &'static str,
    framing: &'static str,
    guides: &'static str,
    date_stamp: &'static str,
    grain: &'static str,
    /// Before the version in the About row.
    version: &'static str,
    off: &'static str,
    ratios: [&'static str; 3],
    /// The ratios in a summary, which has less room than a row.
    ratio_values: [&'static str; 3],
    guide_styles: [&'static str; 5],
    stamp_styles: [&'static str; 7],
    grain_levels: [&'static str; 3],
}

const ENGLISH: Words = Words {
    separator: ": ",
    image_control: "Image Control",
    framing: "Framing",
    guides: "Guides",
    date_stamp: "Date Stamp",
    grain: "Grain",
    version: "Version",
    off: "Off",
    ratios: ["XPan 65:24", "16:9", "4:3"],
    ratio_values: ["XPan", "16:9", "4:3"],
    guide_styles: [
        "Thirds",
        "Center",
        "Golden Ratio",
        "Diagonals",
        "Thirds + Center",
    ],
    stamp_styles: [
        "'YY M D",
        "'YY M D H:MM",
        "M D 'YY",
        "D M 'YY",
        "Small Mono White",
        "Small Mono Black",
        "D M 'YY H:MM",
    ],
    grain_levels: ["Weak", "Medium", "Strong"],
};

const SIMPLIFIED_CHINESE: Words = Words {
    separator: "：",
    image_control: "影像控制",
    framing: "画幅",
    guides: "辅助线",
    date_stamp: "加印日期",
    grain: "颗粒感",
    version: "版本",
    off: "关闭",
    ratios: ["XPan 65:24", "16:9", "4:3"],
    ratio_values: ["XPan", "16:9", "4:3"],
    guide_styles: ["三分法", "中心", "黄金分割", "对角线", "Thirds + Center"],
    stamp_styles: [
        "'YY M D",
        "'YY M D H:MM",
        "M D 'YY",
        "D M 'YY",
        "Small Mono White",
        "Small Mono Black",
        "D M 'YY H:MM",
    ],
    grain_levels: ["弱", "中", "强"],
};

const TRADITIONAL_CHINESE: Words = Words {
    separator: "：",
    image_control: "影像控制",
    framing: "畫幅",
    guides: "輔助線",
    date_stamp: "加印日期",
    grain: "粒紋感",
    version: "版本",
    off: "關閉",
    ratios: ["XPan 65:24", "16:9", "4:3"],
    ratio_values: ["XPan", "16:9", "4:3"],
    guide_styles: ["三分法", "中心", "黃金分割", "對角線", "Thirds + Center"],
    stamp_styles: [
        "'YY M D",
        "'YY M D H:MM",
        "M D 'YY",
        "D M 'YY",
        "Small Mono White",
        "Small Mono Black",
        "D M 'YY H:MM",
    ],
    grain_levels: ["弱", "中", "強"],
};

const JAPANESE: Words = Words {
    // A full-width colon would push "イメージコントロール：" and a 13-character
    // effect name under the icon column. The half-width colon keeps them clear.
    separator: ":",
    image_control: "イメージコントロール",
    framing: "フレーミング",
    guides: "グリッド",
    date_stamp: "日付入れ",
    grain: "粒状感",
    version: "バージョン",
    off: "オフ",
    ratios: ["XPan 65:24", "16:9", "4:3"],
    ratio_values: ["XPan", "16:9", "4:3"],
    guide_styles: ["9分割", "画面中央", "黄金比", "対角線", "Thirds + Center"],
    stamp_styles: [
        "'YY M D",
        "'YY M D H:MM",
        "M D 'YY",
        "D M 'YY",
        "Small Mono White",
        "Small Mono Black",
        "D M 'YY H:MM",
    ],
    grain_levels: ["弱", "中", "強"],
};

fn words(language: Language) -> &'static Words {
    match language {
        Language::English => &ENGLISH,
        Language::SimplifiedChinese => &SIMPLIFIED_CHINESE,
        Language::TraditionalChinese => &TRADITIONAL_CHINESE,
        Language::Japanese => &JAPANESE,
    }
}

/// A run of consecutive string IDs. `runtime/abi.h` defines `TEXT_<name>` as
/// the first ID of each run.
pub struct Run {
    pub name: &'static str,
    pub count: usize,
    pub doc: &'static str,
}

const fn run(name: &'static str, count: usize, doc: &'static str) -> Run {
    Run { name, count, doc }
}

/// The string IDs in table order.
pub const RUNS: &[Run] = &[
    run("IMAGE_CONTROL", 1, ""),
    run("FRAMING", 1, ""),
    run("GUIDES", 1, ""),
    run("DATE_STAMP", 1, ""),
    run("GRAIN", 1, ""),
    run("OFF", 1, ""),
    run("RATIO", 3, "Framing rows 1..3: XPan 65:24, 16:9, 4:3."),
    run(
        "GUIDE",
        5,
        "Guides rows 1..4: thirds, center, golden ratio, diagonals.",
    ),
    run(
        "STAMP",
        7,
        "Date Stamp rows 1..6: four data-back formats and two small mono styles.",
    ),
    run("STAMP_MENU", 2, "Format and Appearance."),
    run(
        "STAMP_LOOK",
        5,
        "Classic, GR1s, GR II, Small Mono White, Small Mono Black.",
    ),
    run(
        "STAMP_FORMAT",
        7,
        "Off and date ordering, independent of appearance.",
    ),
    run("GRAIN_LEVEL", 3, "Grain rows 1..3: weak, medium, strong."),
    run("IMAGE_OFF_SUMMARY", 1, ""),
    run("CROP_NAME", 8, "Capture ratio names."),
    run("CROP_SUMMARY", 8, "Capture ratio summaries."),
    run("FRAMING_SUMMARY", 4, "Summaries for each value, Off first."),
    run("GUIDES_SUMMARY", 6, ""),
    run("STAMP_SUMMARY", 9, ""),
    run("GRAIN_SUMMARY", 4, ""),
    run(
        "IMAGE_PREFIX",
        1,
        "The Image Control summary before an effect name, for the installer.",
    ),
    run("VERSION", 1, "The About row: the GRKit version."),
    run(
        "GRAIN_PERCENT",
        20,
        "GR III/IIIx grain percentages in ascending order.",
    ),
    run("GRAIN_PERCENT_SUMMARY", 21, ""),
    run("BLOOM", 1, "Experimental JPEG bloom."),
    run("HALATION", 1, "Experimental JPEG halation."),
    run("BLACK_FLOOR", 1, ""),
    run("BLACK_FLOOR_LEVEL", 3, "Lift 1 to Lift 3."),
    run("SPLIT_TONING", 1, ""),
    run("SPLIT_MENU", 2, "Preset and strength."),
    run("SPLIT_PRESET", 9, "Off and four tint pairings."),
    run("SPLIT_TONE_PRESET", 12, "Shadow/highlight tint pairs."),
    run("COLOUR_RANGE", 1, ""),
    run("COLOUR_RANGE_NAME", 7, "Reds through magentas."),
    run("COLOUR_SATURATION", 1, ""),
    run(
        "COLOUR_SATURATION_LEVEL",
        26,
        "Neutral, -25%, -50%, -75%, +25%, +50%.",
    ),
    run("COLOUR_BRIGHTNESS", 1, ""),
    run(
        "COLOUR_BRIGHTNESS_LEVEL",
        9,
        "Neutral, -20, -10, +10, +20 luma steps.",
    ),
    run("HIGHLIGHT_ROLLOFF", 1, ""),
    run(
        "STRENGTH_LEVEL",
        20,
        "Twelve steps; original strengths retain saved codes.",
    ),
    run("HIGHLIGHT_ROLLOFF_LEVEL", 3, "Soft, Medium, Strong."),
    run("MENU_GROUP", 3, "Framing, Tone & Colour, Texture & Light."),
    run(
        "MENU_GROUP_SUMMARY",
        3,
        "Group names with an opening arrow.",
    ),
    run(
        "MENU_PARENT",
        3,
        "Short parent names in value-page headings.",
    ),
    run("MENU_BACK", 1, "Explicit parent navigation."),
    run("MENU_LOOKS", 1, ""),
    run("MENU_CROP", 1, ""),
    run("MENU_COLOUR", 1, ""),
    run("SOFT_SHORTCUT", 1, "Opt-in shooting shortcut."),
    run("SHORTCUT_FN", 1, "Normal Fn press in still live view."),
    run("SHORTCUT_SUMMARY", 2, "Off / Fn."),
    run("COLOUR_HUE", 1, ""),
    run("COLOUR_HUE_LEVEL", 9, "Neutral, -20, -10, +10, +20."),
    run("COLOUR_FIRST", 1, "Editing a disabled colour range."),
    run(
        "COLOUR_RANGE_SUMMARY",
        8,
        "Off followed by each editing range.",
    ),
];

/// The first string ID of each run, in table order.
pub fn ids() -> impl Iterator<Item = (&'static Run, usize)> {
    RUNS.iter().scan(0, |next, run| {
        let id = *next;
        *next += run.count;
        Some((run, id))
    })
}

/// The first string ID of a run.
fn id(name: &str) -> usize {
    ids()
        .find(|(run, _)| run.name == name)
        .map(|(_, id)| id)
        .expect("a menu text run")
}

/// The strings of one run in one language. `version` is the GRKit version.
fn run_strings(w: &Words, name: &str, version: &str) -> Vec<String> {
    let summary = |module: &str, value: &str| format!("{module}{}{value}", w.separator);
    let summaries = |module: &str, values: &[&str]| {
        std::iter::once(w.off)
            .chain(values.iter().copied())
            .map(|value| summary(module, value))
            .collect::<Vec<_>>()
    };
    let words = |values: &[&str]| values.iter().map(|value| value.to_string()).collect();
    match name {
        "IMAGE_CONTROL" => words(&[w.image_control]),
        "FRAMING" => words(&[w.framing]),
        "CROP_NAME" => words(&[
            "Native",
            "Legacy",
            "16:9",
            "4:3",
            "5:4",
            "6x7 (7:6)",
            "FP100C",
            "1:1",
        ]),
        "CROP_SUMMARY" => [
            "Native",
            "Legacy",
            "16:9",
            "4:3",
            "5:4",
            "6x7 (7:6)",
            "FP100C",
            "1:1",
        ]
        .iter()
        .map(|s| format!("Crop: {s}"))
        .collect(),
        "GUIDES" => words(&[w.guides]),
        "DATE_STAMP" => words(&[w.date_stamp]),
        "GRAIN" => words(&[w.grain]),
        "BLOOM" => words(&["Bloom"]),
        "HALATION" => words(&["Halation"]),
        "BLACK_FLOOR" => words(&["Black Floor"]),
        "BLACK_FLOOR_LEVEL" => words(&["Lift 1", "Lift 2", "Lift 3"]),
        "SPLIT_TONING" => words(&["Split Toning"]),
        "SPLIT_MENU" => words(&["Preset", "Strength"]),
        "SPLIT_PRESET" => words(&[
            w.off,
            "Warm Cream",
            "Amber / Rose",
            "Cocoa / Cream",
            "Sepia",
            "Warm Shadows",
            "Blue / Amber",
            "Teal / Rose",
            "Cool Shadows",
        ]),
        "SPLIT_TONE_PRESET" => words(&[
            "Blue / Amber 25%",
            "Blue / Amber 50%",
            "Blue / Amber 75%",
            "Blue / Amber 100%",
            "Teal / Rose 25%",
            "Teal / Rose 50%",
            "Teal / Rose 75%",
            "Teal / Rose 100%",
            "Warm Shadows 50%",
            "Warm Shadows 100%",
            "Cool Shadows 50%",
            "Cool Shadows 100%",
        ]),
        "COLOUR_RANGE" => words(&["Colour Range"]),
        "COLOUR_RANGE_NAME" => words(&[
            "Reds", "Oranges", "Yellows", "Greens", "Cyans", "Blues", "Magentas",
        ]),
        "COLOUR_SATURATION" => words(&["Saturation"]),
        "COLOUR_SATURATION_LEVEL" => (-15..=10)
            .map(|n| {
                if n == 0 {
                    "Neutral".into()
                } else {
                    format!("{:+}%", n * 5)
                }
            })
            .collect(),
        "COLOUR_HUE" => words(&["Hue"]),
        "COLOUR_BRIGHTNESS" => words(&["Brightness"]),
        "COLOUR_BRIGHTNESS_LEVEL" => (-4..=4)
            .map(|n| {
                if n == 0 {
                    "Neutral".into()
                } else {
                    format!("{:+}", n * 5)
                }
            })
            .collect(),
        "HIGHLIGHT_ROLLOFF" => words(&["Highlight Roll-off"]),
        "STRENGTH_LEVEL" => (1..=20).map(|n| format!("{}%", n * 5)).collect(),
        "HIGHLIGHT_ROLLOFF_LEVEL" => words(&["Soft", "Medium", "Strong"]),
        "MENU_GROUP" => words(&["Framing", "Tone & Colour", "Texture & Light"]),
        "MENU_GROUP_SUMMARY" => words(&["Framing", "Tone & Colour", "Texture & Light"]),
        "MENU_PARENT" => words(&["Framing", "Tone", "Texture"]),
        "MENU_BACK" => words(&["Back"]),
        "MENU_LOOKS" => words(&["Looks"]),
        "MENU_CROP" => words(&["Crop"]),
        "MENU_COLOUR" => words(&["Colour"]),
        "SOFT_SHORTCUT" => words(&["Soft Shortcut"]),
        "SHORTCUT_FN" => words(&["Fn opens Soft"]),
        "SHORTCUT_SUMMARY" => words(&["Shortcut: Off", "Shortcut: Fn"]),
        "COLOUR_FIRST" => words(&["Choose Colour Range first"]),
        "COLOUR_HUE_LEVEL" => (-4..=4)
            .map(|n| {
                if n == 0 {
                    "Neutral".into()
                } else {
                    format!("{:+}", n * 5)
                }
            })
            .collect(),
        "COLOUR_RANGE_SUMMARY" => words(&[
            "Colour: Off",
            "Colour: Reds",
            "Colour: Oranges",
            "Colour: Yellows",
            "Colour: Greens",
            "Colour: Cyans",
            "Colour: Blues",
            "Colour: Magentas",
        ]),
        "OFF" => words(&[w.off]),
        "RATIO" => words(&w.ratios),
        "GUIDE" => words(&w.guide_styles),
        "STAMP" => words(&w.stamp_styles),
        "STAMP_MENU" => words(&["Format", "Appearance"]),
        "STAMP_LOOK" => words(&[
            "Classic",
            "GR1s Dot Matrix",
            "GR II",
            "Small Mono White",
            "Small Mono Black",
        ]),
        "GRAIN_LEVEL" => words(&w.grain_levels),
        "IMAGE_OFF_SUMMARY" => vec![summary(w.image_control, w.off)],
        "FRAMING_SUMMARY" => summaries(w.framing, &w.ratio_values),
        "GUIDES_SUMMARY" => summaries(w.guides, &w.guide_styles),
        "STAMP_FORMAT" => words(&[
            w.off,
            w.stamp_styles[0],
            w.stamp_styles[1],
            w.stamp_styles[2],
            w.stamp_styles[3],
            w.stamp_styles[6],
            "YYYY.MM.DD",
        ]),
        "STAMP_SUMMARY" => {
            let mut s = summaries(w.date_stamp, &w.stamp_styles);
            s.push(format!("{}{} YYYY.MM.DD", w.date_stamp, w.separator));
            s
        }
        "GRAIN_SUMMARY" => summaries(w.grain, &w.grain_levels),
        "GRAIN_PERCENT" => (1..=20).map(|n| format!("{}%", n * 5)).collect(),
        "GRAIN_PERCENT_SUMMARY" => {
            let mut s = vec![format!("{}{}{}", w.grain, w.separator, w.off)];
            s.extend((1..=20).map(|n| format!("{}{}{}%", w.grain, w.separator, n * 5)));
            s
        }
        "IMAGE_PREFIX" => vec![format!("{}{}", w.image_control, w.separator)],
        "VERSION" => vec![format!("{} {version}", w.version)],
        _ => unreachable!("Unknown menu text run {name}"),
    }
}

/// The strings of one language in the ID order of `runtime/abi.h`. `version`
/// is the GRKit version that the About row shows.
pub fn strings(language: Language, version: &str) -> Vec<String> {
    let w = words(language);
    RUNS.iter()
        .flat_map(|run| run_strings(w, run.name, version))
        .collect()
}

fn units(text: &str) -> Vec<u16> {
    text.encode_utf16().collect()
}

/// The label table: for each language, `STRINGS` u16 byte offsets from the
/// table start, then the NUL-terminated UTF-16LE strings. `version` is the
/// GRKit version that the About row shows.
pub fn table(version: &str) -> Result<Vec<u8>> {
    ensure!(
        !version.is_empty() && version.bytes().all(|byte| byte.is_ascii_graphic()),
        "The version {version:?} is not printable ASCII"
    );
    let languages = Language::ALL;
    let mut offsets = Vec::new();
    let mut strings_area: Vec<u16> = Vec::new();
    let mut shared = std::collections::BTreeMap::new();
    let strings_start = languages.len() * STRINGS * 2;
    for language in languages {
        let list = strings(language, version);
        ensure!(
            list.len() == STRINGS,
            "Menu text has {} strings",
            list.len()
        );
        for text in &list {
            let encoded = units(text);
            ensure!(
                encoded.len() < SUMMARY_UNITS,
                "Menu text is too long: {text:?}"
            );
            // Labels shared across languages or pages need only one copy.
            // The table still has an offset for every symbolic text ID.
            let offset = if let Some(offset) = shared.get(&encoded) {
                *offset
            } else {
                let offset = u16::try_from(strings_start + strings_area.len() * 2)?;
                strings_area.extend(&encoded);
                strings_area.push(0);
                shared.insert(encoded, offset);
                offset
            };
            offsets.push(offset);
        }
        let prefix = units(&list[id("IMAGE_PREFIX")]).len();
        ensure!(
            prefix + NAME_UNITS < SUMMARY_UNITS,
            "The Image Control prefix leaves no room for an effect name"
        );
    }
    let mut out: Vec<u8> = offsets.iter().flat_map(|v| v.to_le_bytes()).collect();
    out.extend(strings_area.iter().flat_map(|v| v.to_le_bytes()));
    ensure!(
        out.len() <= TEXT_SUMMARY,
        "Menu text table exceeds {TEXT_SUMMARY} bytes"
    );
    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    const VERSION: &str = "0.0.0";

    #[test]
    fn the_table_fits_and_points_at_each_string() {
        let table = table(VERSION).unwrap();
        for (index, language) in Language::ALL.into_iter().enumerate() {
            for (id, expected) in strings(language, VERSION).iter().enumerate() {
                let at = index * STRINGS + id;
                let offset = u16::from_le_bytes([table[at * 2], table[at * 2 + 1]]) as usize;
                let read: Vec<u16> = table[offset..]
                    .chunks_exact(2)
                    .map(|pair| u16::from_le_bytes([pair[0], pair[1]]))
                    .take_while(|unit| *unit != 0)
                    .collect();
                assert_eq!(String::from_utf16(&read).unwrap(), *expected);
            }
        }
    }

    #[test]
    fn each_run_has_its_declared_number_of_strings() {
        for language in Language::ALL {
            for run in RUNS {
                assert_eq!(
                    run_strings(words(language), run.name, VERSION).len(),
                    run.count,
                    "{} in {language:?}",
                    run.name
                );
            }
        }
        assert_eq!(RUNS.iter().map(|run| run.count).sum::<usize>(), STRINGS);
    }

    #[test]
    fn english_keeps_the_menu_text_of_earlier_builds() {
        let english = strings(Language::English, VERSION);
        assert_eq!(english[1], "Framing");
        assert_eq!(english[id("FRAMING_SUMMARY") + 1], "Framing: XPan");
        assert_eq!(english[id("IMAGE_OFF_SUMMARY")], "Image Control: Off");
        assert_eq!(english[id("IMAGE_PREFIX")], "Image Control: ");
    }

    #[test]
    fn the_about_row_shows_the_version_in_each_language() {
        assert_eq!(
            strings(Language::English, "1.2.3")[id("VERSION")],
            "Version 1.2.3"
        );
        assert_eq!(
            strings(Language::SimplifiedChinese, "1.2.3")[id("VERSION")],
            "版本 1.2.3"
        );
        assert_eq!(
            strings(Language::TraditionalChinese, "1.2.3")[id("VERSION")],
            "版本 1.2.3"
        );
        assert_eq!(
            strings(Language::Japanese, "1.2.3")[id("VERSION")],
            "バージョン 1.2.3"
        );
    }

    #[test]
    fn the_table_rejects_a_version_that_the_menu_cannot_show() {
        assert!(table("").is_err());
        assert!(table("1.0 beta").is_err());
        assert!(table("1.0.0-测试").is_err());
        assert!(table(&"9".repeat(SUMMARY_UNITS)).is_err());
    }
}
