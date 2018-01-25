#include <cfloat>

#include "BinaryIO.h"
#include "CvrStgFile.h"
#include "Edge.h"
#include "EmbData.h"
#include "Embedderlib.h"
#include "Graph.h"
#include "Matching.h"
#include "BitString.h"
#include "MatchingAlgorithm.h"
#include "Selector.h"

EmbedderLib::EmbedderLib(std::string CoverFileName, std::string EncodedFileName, std::string Pass, EncryptionAlgorithm algorithm, EncryptionMode encryptionMode)
{

    _CoverFileName = CoverFileName;
    _EncodedFileName = EncodedFileName;
    // read embfile
    Arguments* a = new Arguments();


    std::vector<BYTE> emb ;
    BinaryIO embio (EncodedFileName, BinaryIO::READ) ;
    while (!embio.eof()) {
        emb.push_back (embio.read8()) ;
    }
    embio.close() ;

    // create bitstring to be embedded
    EmbData embdata (EmbData::EMBED, Pass, EncodedFileName) ;
    embdata.setEncAlgo (algorithm);
    embdata.setEncMode (encryptionMode);
    embdata.setCompression (a->Default_Compression) ;
    embdata.setChecksum (a->Default_Checksum) ;
    embdata.setData (emb) ;
    _ToEmbed = embdata.getBitString();

    // read cover-/stego-file
    _CoverFile = CvrStgFile::readFile(CoverFileName) ;

    // create graph
    _ToEmbed.setArity (_CoverFile->getEmbValueModulus()) ;
    if ((_ToEmbed.getNAryLength() * _CoverFile->getSamplesPerVertex()) > _CoverFile->getNumSamples()) {
        throw SteghideError (_("the cover file is too short to embed the data.")) ;
    }

    Selector sel (_CoverFile->getNumSamples(), Pass) ;
    _theGraph = new Graph(_CoverFile, _ToEmbed, sel) ;

}
void EmbedderLib::Embed()
{
    const Matching* M = calculateMatching() ;

    // embed matched edges
    const std::list<Edge*> medges = M->getEdges() ;
    for (std::list<Edge*>::const_iterator it = medges.begin() ; it != medges.end() ; it++) {
        embedEdge (*it) ;
    }

    // embed exposed vertices
    const std::list<Vertex*> *expvertices = M->getExposedVerticesLink() ;
    for (std::list<Vertex*>::const_iterator it = expvertices->begin() ; it != expvertices->end() ; it++) {
        embedExposedVertex (*it) ;
    }

    delete M ;

    // write stego file
    _CoverFile->transform (_CoverFileName) ;

    bool displaydone = false ;
    if (_CoverFile->is_std()) {
        Message ws (_("writing stego file to standard output... ")) ;
        ws.printMessage() ;
    }
    else {
        if (_EncodedFileName != _CoverFileName) {
            Message ws (_("writing stego file \"%s\"... "), _CoverFile->getName().c_str()) ;
            ws.setNewline (false) ;
            ws.printMessage() ;
            displaydone = true ;
        }
    }

    _CoverFile->write() ;

    if (displaydone) {
        Message wsd (_("done")) ;
        wsd.printMessage() ;
    }
}

const Matching* EmbedderLib::calculateMatching()
{
    Matching* matching = new Matching(_theGraph) ;

    std::vector<MatchingAlgorithm*> MatchingAlgos = _CoverFile->getMatchingAlgorithms (_theGraph, matching) ;

    for (std::vector<MatchingAlgorithm*>::const_iterator ait = MatchingAlgos.begin() ; ait != MatchingAlgos.end() ; ait++) {

        (*ait)->setGoal (Args.Goal.getValue()) ;
        (*ait)->run() ;
        delete *ait ;
    }

    if (Args.Check.getValue()) {
        if (!matching->check()) {
            CriticalWarning w ("integrity checking of matching data structures failed!") ; // TODO: internationalize this
            w.printMessage() ;
        }
    }

    return matching ;
}

void EmbedderLib::embedEdge (Edge *e)
{
    Vertex *v1 = e->getVertex1() ;
    Vertex *v2 = e->getVertex2() ;

    _CoverFile->replaceSample (e->getSamplePos(v1), e->getReplacingSampleValue (v1)) ;
    _CoverFile->replaceSample (e->getSamplePos(v2), e->getReplacingSampleValue (v2)) ;
}

void EmbedderLib::embedExposedVertex (Vertex *v)
{
    SamplePos samplepos = 0 ;
    SampleValue *newsample = NULL ;
    float mindistance = FLT_MAX ;
    for (unsigned short i = 0 ; i < _CoverFile->getSamplesPerVertex() ; i++) {
        SampleValue *curold = v->getSampleValue(i) ;
        SampleValue *curnew = v->getSampleValue(i)->getNearestTargetSampleValue(v->getTargetValue(i)) ;
        if (curold->calcDistance (curnew) < mindistance) {
            samplepos = v->getSamplePos(i) ;
            newsample = curnew ;
            mindistance = curold->calcDistance (curnew) ;
        }
        else {
            delete curnew;
        }
    }

#ifdef DEBUG
    printDebug (1, "embedding vertex with label %lu by changing sample position %lu.", v->getLabel(), samplepos) ;
#endif

    EmbValue oldev = _CoverFile->getEmbeddedValue (samplepos) ;
    _CoverFile->replaceSample (samplepos, newsample) ;
    myassert (oldev != _CoverFile->getEmbeddedValue (samplepos)) ;
    delete newsample ;
}
