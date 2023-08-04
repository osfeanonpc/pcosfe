#include "pch.h"
#include "DS.h"
#include "MTX.h"
#include "OSFEpsDrv.h"

LIST_ENTRY* DSHolder::deleteRecord() {
	auto item = RemoveHeadList(&m_ItemsHead);
	if (item == &m_ItemsHead)
		return nullptr;

	m_Count--;
	return item;
}

void DSHolder::insertRecord(LIST_ENTRY* entry) {
	if (m_Count == m_MaxCount) {
		auto head = RemoveHeadList(&m_ItemsHead);
		ExFreePool(CONTAINING_RECORD(head, FullItem<ItemHeader>, Entry));
		m_Count--;
	}

	InsertTailList(&m_ItemsHead, entry);
	m_Count++;
}

void DSHolder::init(ULONG maxCount) {
	InitializeListHead(&m_ItemsHead);
	m_Count = 0;
	if (maxCount > 0x1000)
		m_MaxCount = maxCount;
	else
		m_MaxCount = 0x1000;
}

void DSHolder::insertHeadRecord(LIST_ENTRY* entry) {
	InsertHeadList(&m_ItemsHead, entry);
	m_Count++;
}

