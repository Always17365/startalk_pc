//
// Created by may on 2019-03-18.
//

#ifndef STALK_V2_PLAYAUDIOSOUND_H
#define STALK_V2_PLAYAUDIOSOUND_H


namespace st {
    namespace mac {
        namespace util {
            class PlayAudioSound {
            public:
                static void PlaySound(const char* path);
                static void removeSound(const char* path);
            };
        }
    }
}


#endif //STALK_V2_PLAYAUDIOSOUND_H
