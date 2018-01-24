#include "BinaryIO.h"
#include "CvrStgFile.h"
#include "EmbData.h"
#include "Embedderlib.h"

EmbedderLib::EmbedderLib(std::string CoverFileName, std::string EncodedFileName, std::string Pass, EncryptionAlgorithm algorithm, EncryptionMode encryptionMode)
{
    // read embfile
    Arguments* a = new Arguments();

    std::vector<BYTE> emb ;
    BinaryIO embio (EncodedFileName, BinaryIO::READ) ;
    while (!embio.eof()) {
        emb.push_back (embio.read8()) ;
    }
    embio.close() ;

    // create bitstring to be embedded
    std::string fn = EncodedFileName;
    EmbData embdata (EmbData::EMBED, Pass, fn) ;
    embdata.setEncAlgo (algorithm);
    embdata.setEncMode (encryptionMode);
    embdata.setCompression (a->Default_Compression) ;
    embdata.setChecksum (a->Default_Checksum) ;
    embdata.setData (emb) ;
    ToEmbed = embdata.getBitString() ;

    // read cover-/stego-file
    CvrStgFile* stegfile = CvrStgFile::readFile(CoverFileName) ;

    ToEmbed.setArity (Globs.TheCvrStgFile->getEmbValueModulus()) ;
    if ((ToEmbed.getNAryLength() * Globs.TheCvrStgFile->getSamplesPerVertex()) > Globs.TheCvrStgFile->getNumSamples()) {
        throw SteghideError (_("the cover file is too short to embed the data.")) ;
    }
}
