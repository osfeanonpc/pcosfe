#pragma once


struct DSHolder {
	void init(ULONG maxItems);

	void insertRecord(LIST_ENTRY* entry);
	void insertHeadRecord(LIST_ENTRY* entry);
	LIST_ENTRY* deleteRecord();

private:
	LIST_ENTRY m_ItemsHead;
	ULONG m_Count;
	ULONG m_MaxCount;
};

