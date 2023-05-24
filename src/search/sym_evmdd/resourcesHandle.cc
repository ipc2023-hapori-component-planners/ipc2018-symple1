// Copyright 30.06.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include "resourcesHandle.h"
#include "meddly_extensions/userDefiniedOperators.h"
#include "../globals.h"
#include "../timer.h"

void printMemoryState(MEDDLY::forest* f) {
  std::cout << "--------- Memory state -------" << std::endl;
  std::cout << " - Nodes of forest: " << f->getCurrentNumNodes() << std::endl;
  std::cout << " - MemUsed: " << f->getCurrentMemoryUsed() / 1000.00 / 1000.00
  << "MB" << std::endl;
  std::cout << " - MemAllocated: "
  << f->getCurrentMemoryAllocated() / 1024.00 / 1024.00 << "MB" << std::endl;
  std::cout << " - " << std::flush;
  print_used_memory();
  std::cout << " - #CT entries: "
  << unsigned(getComputeTable(f)->getStats().numEntries) << " ~ "
  << unsigned(getComputeTable(f)->getStats().numEntries) * 6 * 4 / 1024.00
  / 1024.00 << "MB" << std::endl;
  std::cout << " - #CT Max entries: " << unsigned(getComputeTable(f)->maxSize)
  << " ~ " << unsigned(getComputeTable(f)->maxSize) * 6 * 4 / 1024.00 / 1024.00
  << "MB" << std::endl;
  std::cout << " - " << std::flush;
  print_used_memory();
  std::cout << "-----------------------------" << std::endl;
}

long getNumOfEdges(MEDDLY::forest* f) {
  long result = 0;
  auto expertF = static_cast<MEDDLY::expert_forest*>(f);
  long ap = 0;
  FILE* tempFile;
  tempFile = fopen("tmpfile", "w");
  for (long n = 0; n < f->getCurrentNumNodes(); n++) {
    MEDDLY::FILE_output out(tempFile);
    if (!expertF->showNode(out, ap)) {
      ap++;
      n--;
      continue;
    }
    MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(expertF, ap,
    true);
    const int resultLevel = expertF->getNodeLevel(ap);
    const int resultSize = expertF->getLevelSize(resultLevel);
    for (int i = 0; i < resultSize; i++) {
      result++;
    }
    MEDDLY::unpacked_node::recycle(A);
    ap++;
  }
  return result;
}

int getNumOfSuppressableEdges(MEDDLY::forest* f, int edgeValue) {
  int result = 0;
  auto expertF = static_cast<MEDDLY::expert_forest*>(f);
  long ap = 0;
  FILE* tempFile;
  tempFile = fopen("tmpfile", "w");
  for (long n = 0; n < f->getCurrentNumNodes(); n++) {
    MEDDLY::FILE_output out(tempFile);
    if (!expertF->showNode(out, ap)) {
      ap++;
      n--;
      continue;
    }
    MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(expertF, ap,
    true);
    const int resultLevel = expertF->getNodeLevel(ap);
    const int resultSize = expertF->getLevelSize(resultLevel);
    for (int i = 0; i < resultSize; i++) {
      if (A->d(i) == -1 && A->ei(i) == edgeValue) {
        result++;
      }
    }
    MEDDLY::unpacked_node::recycle(A);
    ap++;
  }
  return result;
}

