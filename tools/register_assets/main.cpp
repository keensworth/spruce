#include "AssetRegisterer.h"

using namespace spr::tools;

// parses .s*** files into:
//      asset_ids.h - stores models/subresources with enums/ids
//      asset_manifest.json - stores all resource metadata of
//                            present resources
//
// starts from .smdl files and parses depth-first

int main(int argc, char **argv){
    AssetRegisterer ar = AssetRegisterer();
    ar.registerDirectory("../data/");
}