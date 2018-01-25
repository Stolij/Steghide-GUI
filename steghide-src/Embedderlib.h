#ifndef EMBEDDERLIB_H
#define EMBEDDERLIB_H

#include "BitString.h"
#include "EncryptionAlgorithm.h"
#include "EncryptionMode.h"
#include "Matching.h"

#include <string>
class Graph ;

class EmbedderLib {
    public:
        EmbedderLib(std::string CoverFileName, std::string EncodedFileName, std::string pass, EncryptionAlgorithm algorithm, EncryptionMode encryptionmode);

        void Embed();
    private:
        const Matching *EmbedderLib::calculateMatching ();
        void EmbedderLib::embedEdge (Edge *e);
        void EmbedderLib::embedExposedVertex (Vertex *v);

        std::string _EncodedFileName;
        std::string _CoverFileName;

        Graph *_theGraph;

        BitString _ToEmbed;
        CvrStgFile* _CoverFile;
};


#endif // EMBEDDERLIB_H
