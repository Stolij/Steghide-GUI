#ifndef EMBEDDERLIB_H
#define EMBEDDERLIB_H

#include "BitString.h"
#include "EncryptionAlgorithm.h"
#include "EncryptionMode.h"

#include <string>

class EmbedderLib {
    public:
        EmbedderLib(std::string CoverFileName, std::string EncodedFileName, std::string pass, EncryptionAlgorithm algorithm, EncryptionMode encryptionmode);

        void embed();
    private:
        BitString ToEmbed;
};


#endif // EMBEDDERLIB_H
