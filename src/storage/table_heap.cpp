#include "storage/table_heap.h"
#include "common/table_page.h"

TableHeap::TableHeap(BufferPoolManager* bpm, page_id_t first_page_id)
    : bpm(bpm), first_page_id(first_page_id), last_page_id(INVALID_PAGE_ID) {
  if (first_page_id == INVALID_PAGE_ID) {
    Page* new_page = bpm->NewPage(&first_page_id);
    this->first_page_id = first_page_id;
    TablePage* page = reinterpret_cast<TablePage*>(new_page);
    page->Init(first_page_id, INVALID_PAGE_ID);
    last_page_id = first_page_id;
    bpm->UnpinPage(first_page_id, false);
  } else {
    last_page_id = first_page_id;
  }
}

bool TableHeap::InsertTuple(const Tuple& tuple, RID* rid) {
  std::lock_guard<std::mutex> lock(latch);
  auto* page = reinterpret_cast<TablePage*>(bpm->FetchPage(last_page_id));
  if (page == nullptr) {
    return false;
  }

  if (page->InsertTuple(tuple, rid)) {
    bpm->UnpinPage(page->GetPageId(), true);
    return true;
  }

  page_id_t next_page_id = INVALID_PAGE_ID;
  Page* new_raw_page = bpm->NewPage(&next_page_id);
  if (new_raw_page == nullptr) {
    bpm->UnpinPage(page->GetPageId(), false);
    return false;
  }

  auto* next_page = reinterpret_cast<TablePage*>(new_raw_page);

  next_page->Init(next_page_id, page->GetPageId());
  page->SetNextPageId(next_page_id);

  last_page_id = next_page_id;

  bool success = next_page->InsertTuple(tuple, rid);

  bpm->UnpinPage(page->GetPageId(), true);
  bpm->UnpinPage(next_page->GetPageId(), true);

  return success;
}

bool TableHeap::GetTuple(const RID& rid, Tuple* tuple) {
  TablePage* page = reinterpret_cast<TablePage*>(bpm->FetchPage(rid.GetPageId()));
  if (page == nullptr) {
    return false;
  }

  bool success = page->GetTuple(rid, tuple);
  bpm->UnpinPage(page->GetPageId(), false);
  return success;
}

bool TableHeap::DeleteTuple(const RID& rid) {
  TablePage* page = reinterpret_cast<TablePage*>(bpm->FetchPage(rid.GetPageId()));
  if (page == nullptr) {
    return false;
  }

  bool success = page->MarkDelete(rid);
  if (!success) {
    bpm->UnpinPage(page->GetPageId(), true);
    return false;
  }
  success = bpm->UnpinPage(page->GetPageId(), true);
  return success;
}