// This is a function removed from Face.h to get rid of the warning every time
// you compiled a file that used Affectiva headers.

#include <Face.h>

std::string affdex::EmojiToString(Emoji emoji) {
        std::string ret = "";
        switch (emoji)
        {
            case Emoji::Relaxed:
                ret = "Relaxed";
                break;
            case Emoji::Smiley:
                ret = "Smiley";
                break;
            case Emoji::Laughing:
                ret = "Laughing";
                break;
            case Emoji::Kissing:
                ret = "Kissing";
                break;
            case Emoji::Disappointed:
                ret = "Disappointed";
                break;
            case Emoji::Rage:
                ret = "Rage";
                break;
            case Emoji::Smirk:
                ret = "Smirk";
                break;
            case Emoji::Wink:
                ret = "Wink";
                break;
            case Emoji::StuckOutTongueWinkingEye:
                ret = "StuckOutTongueWinkingEye";
                break;
            case Emoji::StuckOutTongue:
                ret = "StuckOutTongue";
                break;
            case Emoji::Flushed:
                ret = "Flushed";
                break;
            case Emoji::Scream:
                ret = "Scream";
                break;
            case Emoji::Unknown:
            default:
                ret = "Unknown";
                break;
        };
        return ret;
}
