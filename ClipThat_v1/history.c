#include "history.h"
#include <stdlib.h>
#include <wchar.h>

static wchar_t* dupString(const wchar_t* source)
{
	if (source == NULL)
		return NULL;
	size_t length = wcslen(source);
	wchar_t* copy = malloc((length + 1) * sizeof(*copy));
	if (copy == NULL)
		return NULL;
	wcscpy_s(copy, length + 1, source);
	return copy;
}
static void deleteEntry(ClipboardEntry* entry) {
	if (entry != NULL) {
		free(entry->text);
		entry->text = NULL;
		free(entry);
	}
}

void historyClear(ClipboardEntry** head) {
	if (head == NULL || *head == NULL) { return; }
	ClipboardEntry* current = *head;
	while (current != NULL) {
		ClipboardEntry* next = current->next;
		deleteEntry(current);
		current = next; 
	}
	*head = NULL;
}

static ClipboardEntry* historyDetachDuplicate(ClipboardEntry** head, const wchar_t* text) {
	if (head == NULL || *head == NULL || text == NULL || *text == L'\0') { return NULL; }
	ClipboardEntry* previous = NULL;
	ClipboardEntry* current = *head;
	while (current != NULL) {
		if (wcscmp(current->text, text) == 0) {
			if (previous == NULL) {
				*head = current->next;
			}
			else {
				previous->next = current->next;
			}
			current->next = NULL;
			return current;
		}
		previous = current;
		current = current->next;
	}
	return NULL;
}

bool historyAdd(ClipboardEntry** head, const wchar_t* text)
{
	if (head == NULL || text == NULL || *text == L'\0') { return false; }

	ClipboardEntry* existing = historyDetachDuplicate(head, text);
	
	if (existing != NULL) {
		existing->next = *head;
		*head = existing;
		return true;
	}

	ClipboardEntry* entry = calloc(1, sizeof(*entry));
	if (entry == NULL) { return false; }

	entry->text = dupString(text);
	if (entry->text == NULL) {
		free(entry);
		return false;
	}
	entry->length = wcslen(entry->text);
	entry->next = *head;
	*head = entry;
	return true;
}

void historyLimit(ClipboardEntry** head, size_t maxCount) {
	if (head == NULL || *head == NULL || maxCount == 0) { return; }
	ClipboardEntry* current = *head;
	size_t count = 1;
	while (current->next != NULL && count < maxCount) {
		current = current->next;
		count++;
	}
	ClipboardEntry* overflow = current->next;
	current->next = NULL;
	while (overflow != NULL) {
		ClipboardEntry* next = overflow->next;
		deleteEntry(overflow);
		overflow = next;
	}
}