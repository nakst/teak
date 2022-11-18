#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
#define UI_WINDOWS
#else
#define UI_LINUX 
#endif

#define UI_IMPLEMENTATION
#include "luigi2.h"

#include "../native_interface.h"

typedef struct PainterWrapper {
	UIPainter *painter;
} PainterWrapper;

typedef struct ElementWrapper {
	UIElement *element;
	uintptr_t referenceCount;
	intptr_t messageUser;
	intptr_t messageClass;
	intptr_t contextUser;
	intptr_t contextClass;
	char *cStringStore;
} ElementWrapper;

// TODO How should this be done properly?
struct ExecutionContext *contextForCallback;

intptr_t wrapPainterInAnytype;
intptr_t wrapTableGetItemInAnytype;
intptr_t wrapKeyTypedInAnytype;

bool ReturnRectangle(struct ExecutionContext *context, UIRectangle rect) {
	return ScriptReturnStructInl(context, 4, rect.l, false, rect.r, false, rect.t, false, rect.b, false);
}

bool ReturnCString(struct ExecutionContext *context, const char *cString) {
	return ScriptReturnString(context, cString, strlen(cString));
}

void PainterWrapperClose(struct ExecutionContext *context, void *wrapper) {
	free(wrapper);
}

void ElementWrapperClose(struct ExecutionContext *context, void *_wrapper) {
	ElementWrapper *wrapper = (ElementWrapper *) _wrapper;

	wrapper->referenceCount--;

	if (wrapper->referenceCount == 0) {
		if (wrapper->messageUser)  ScriptHeapRefClose(context, wrapper->messageUser);
		if (wrapper->messageClass) ScriptHeapRefClose(context, wrapper->messageClass);
		if (wrapper->contextUser)  ScriptHeapRefClose(context, wrapper->contextUser);
		if (wrapper->contextClass) ScriptHeapRefClose(context, wrapper->contextClass);
		free(wrapper->cStringStore);
		UI_ASSERT(!wrapper->element);
		free(wrapper);
	}
}

bool WrapperOpen(struct ExecutionContext *context, ElementWrapper *wrapper, intptr_t *handle) {
	UI_ASSERT(wrapper->referenceCount);
	wrapper->referenceCount++;
	return ScriptCreateHandle(context, wrapper, ElementWrapperClose, handle);
}

int WrapperMessage(UIElement *element, UIMessage message, int di, void *dp) {
	ElementWrapper *wrapper = (ElementWrapper *) element->cp;

	if (!wrapper) {
		return 0;
	}

	int64_t returnValue = 0;
	intptr_t handlers[2] = { wrapper->messageUser, wrapper->messageClass };

	for (int handler = 0; handler < 2 && !returnValue; handler++) {
		// TODO How to handle failures in here?

		if (!handlers[handler]) continue;
		
		intptr_t elementHandle = -1;
		if (!WrapperOpen(contextForCallback, wrapper, &elementHandle)) exit(1);

		intptr_t rawHandle = 0;
		intptr_t dataHandle = 0;
		void *dataWrapper = NULL;

		if (message == UI_MSG_PAINT) {
			PainterWrapper *painterWrapper = (PainterWrapper *) calloc(1, sizeof(PainterWrapper));
			dataWrapper = painterWrapper;
			painterWrapper->painter = (UIPainter *) dp;
			if (!ScriptCreateHandle(contextForCallback, painterWrapper, PainterWrapperClose, &rawHandle)) exit(1);
			int64_t parameters[1] = { (int64_t) rawHandle };
			bool managedParameters[4] = { true };
			if (!ScriptRunCallback(contextForCallback, wrapPainterInAnytype, parameters, managedParameters, 1, &dataHandle, true)) exit(1);
			ScriptHeapRefClose(contextForCallback, rawHandle);
		} else if (message == UI_MSG_TABLE_GET_ITEM) {
			UITableGetItem *m = (UITableGetItem *) dp;
			int64_t fields[4] = { (int64_t) m->index, (int64_t) m->column, (int64_t) 0, (int64_t) 0 };
			bool managedFields[4] = { false, false, true, false };
			if (!ScriptCreateStruct(contextForCallback, fields, managedFields, 4, &rawHandle)) exit(1);
			int64_t parameters[1] = { (int64_t) rawHandle };
			bool managedParameters[4] = { true };
			if (!ScriptRunCallback(contextForCallback, wrapTableGetItemInAnytype, parameters, managedParameters, 1, &dataHandle, true)) exit(1);
		} else if (message == UI_MSG_KEY_TYPED || message == UI_MSG_KEY_RELEASED) {
			UIKeyTyped *m = (UIKeyTyped *) dp;
			intptr_t stringHandle = 0;
			if (m->textBytes && !ScriptCreateString(contextForCallback, m->text, m->textBytes, &stringHandle)) exit(1);
			int64_t fields[2] = { (int64_t) stringHandle, (int64_t) m->code };
			bool managedFields[2] = { true, false };
			if (!ScriptCreateStruct(contextForCallback, fields, managedFields, 2, &rawHandle)) exit(1);
			int64_t parameters[1] = { (int64_t) rawHandle };
			bool managedParameters[4] = { true };
			if (!ScriptRunCallback(contextForCallback, wrapKeyTypedInAnytype, parameters, managedParameters, 1, &dataHandle, true)) exit(1);
			ScriptHeapRefClose(contextForCallback, rawHandle);
			if (stringHandle) ScriptHeapRefClose(contextForCallback, stringHandle);
		}

		int64_t parameters[4] = { (int64_t) elementHandle, (int64_t) message, (int64_t) di, (int64_t) dataHandle };
		bool managedParameters[4] = { true, false, false, true };
		if (!ScriptRunCallback(contextForCallback, handlers[handler], parameters, managedParameters, 4, &returnValue, false)) exit(1);

		if (message == UI_MSG_PAINT) {
			((PainterWrapper *) dataWrapper)->painter = NULL;
		} else if (message == UI_MSG_TABLE_GET_ITEM) {
			UITableGetItem *m = (UITableGetItem *) dp;
			const char *text;
			size_t textBytes;
			int32_t isSelected;
			if (!ScriptStructReadString(contextForCallback, rawHandle, 2, (const void **) &text, &textBytes)) exit(1);
			if (!ScriptStructReadInt32(contextForCallback, rawHandle, 3, &isSelected)) exit(1);
			ScriptHeapRefClose(contextForCallback, rawHandle);
			returnValue = m->bufferBytes > textBytes ? textBytes : m->bufferBytes;
			memcpy(m->buffer, text, returnValue);
		}

		ScriptHeapRefClose(contextForCallback, elementHandle);
	}

	if (message == UI_MSG_DESTROY) {
		element->cp = NULL;
		wrapper->element = NULL;
		returnValue = 0;
		ElementWrapperClose(contextForCallback, wrapper);
	}

	return returnValue;
}

ElementWrapper *WrapperCreate(UIElement *element) {
	if (element->cp) {
		ElementWrapper *wrapper = (ElementWrapper *) element->cp;
		UI_ASSERT(wrapper->referenceCount);
		wrapper->referenceCount++;
		return wrapper;
	} else {
		ElementWrapper *wrapper = (ElementWrapper *) UI_CALLOC(sizeof(ElementWrapper));
		element->cp = wrapper;
		element->messageUser = WrapperMessage;
		wrapper->element = element;
		wrapper->referenceCount = 2; // One returned here, one held by the element until MSG_DESTROY.
		return wrapper;
	}
}

LIBRARY_EXPORT bool ScriptExt_InternalInitialise(struct ExecutionContext *context) {
	if (!ScriptParameterHeapRef(context, &wrapPainterInAnytype)) return false;
	if (!ScriptParameterHeapRef(context, &wrapTableGetItemInAnytype)) return false;
	if (!ScriptParameterHeapRef(context, &wrapKeyTypedInAnytype)) return false;
	UIInitialise();
	return true;
}

LIBRARY_EXPORT bool ScriptExtMessageLoop(struct ExecutionContext *context) {
	if (contextForCallback) {
		fprintf(stderr, "Error: Already running the message loop!\n");
		return false;
	}

	contextForCallback = context;
	ScriptReturnInt(context, UIMessageLoop());
	contextForCallback = NULL;
	return true;
}

LIBRARY_EXPORT bool ScriptExtWindowCreate(struct ExecutionContext *context) {
	ElementWrapper *owner;
	uint32_t flags;
	char *title = NULL;
	int32_t width, height;

	bool success = ScriptParameterHandle(context, (void **) &owner)
		&& ScriptParameterUint32(context, &flags)
		&& ScriptParameterCString(context, &title)
		&& ScriptParameterInt32(context, &width)
		&& ScriptParameterInt32(context, &height);

	if (success) {
		UIWindow *window = UIWindowCreate(owner ? (UIWindow *) owner->element : NULL, flags, title, width, height);
		success = ScriptReturnHandle(context, WrapperCreate(&window->e), ElementWrapperClose);
	} else {
		success = ScriptReturnHandle(context, NULL, NULL);
	}

	free(title);
	return success;
}

LIBRARY_EXPORT bool ScriptExtElementCreate(struct ExecutionContext *context) {
	ElementWrapper *parent;
	uint32_t flags;
	intptr_t messageClass;

	if (!ScriptParameterHandle(context, (void **) &parent)
			|| !ScriptParameterUint32(context, &flags)
			|| !ScriptParameterHeapRef(context, &messageClass)) {
		return false;
	}

	UIElement *element = UIElementCreate(sizeof(UIElement), parent ? parent->element : NULL, flags, WrapperMessage, "Custom");
	ElementWrapper *wrapper = WrapperCreate(element);
	wrapper->messageClass = messageClass;
	element->messageClass = NULL;
	return ScriptReturnHandle(context, wrapper, ElementWrapperClose);
}

#define SCRIPT_EXT_GET_FIELD(name, type, field, fieldType) \
LIBRARY_EXPORT bool ScriptExt##name(struct ExecutionContext *context) { \
	ElementWrapper *element; \
	return ScriptParameterScan(context, "h", (void **) &element) && ScriptReturn##fieldType(context, ((type *) element->element)->field); \
}
SCRIPT_EXT_GET_FIELD(WindowGetCursorX, UIWindow, cursorX, Int);
SCRIPT_EXT_GET_FIELD(WindowGetCursorY, UIWindow, cursorY, Int);
SCRIPT_EXT_GET_FIELD(WindowGetPressedMouseButton, UIWindow, pressedButton, Int);
SCRIPT_EXT_GET_FIELD(WindowGetScale, UIWindow, scale, Double);
SCRIPT_EXT_GET_FIELD(WindowGetTextboxModifiedFlag, UIWindow, textboxModifiedFlag, Int);
SCRIPT_EXT_GET_FIELD(WindowIsAltHeld, UIWindow, alt, Int);
SCRIPT_EXT_GET_FIELD(WindowIsCtrlHeld, UIWindow, ctrl, Int);
SCRIPT_EXT_GET_FIELD(WindowIsShiftHeld, UIWindow, shift, Int);
SCRIPT_EXT_GET_FIELD(ElementGetFlags, UIElement, flags, Int);
SCRIPT_EXT_GET_FIELD(TextboxGetSelectionFrom, UITextbox, carets[0], Int);
SCRIPT_EXT_GET_FIELD(TextboxGetSelectionTo, UITextbox, carets[1], Int);
SCRIPT_EXT_GET_FIELD(SliderGetPosition, UISlider, position, Double);
SCRIPT_EXT_GET_FIELD(CheckboxGetCheck, UICheckbox, check, Int);

#define SCRIPT_EXT_LABELLED_ELEMENT_CREATE(name) \
LIBRARY_EXPORT bool ScriptExt##name##Create(struct ExecutionContext *context) { \
	ElementWrapper *parent; \
	uint32_t flags; \
	const char *string; \
	size_t stringBytes; \
	bool success = ScriptParameterHandle(context, (void **) &parent) \
		&& ScriptParameterUint32(context, &flags) \
		&& ScriptParameterString(context, (const void **) &string, &stringBytes); \
	if (success) { \
		UI##name *element = UI##name##Create(parent ? parent->element : NULL, flags, string, stringBytes); \
		return ScriptReturnHandle(context, WrapperCreate(&element->e), ElementWrapperClose); \
	} else { \
		return ScriptReturnHandle(context, NULL, NULL); \
	} \
}
SCRIPT_EXT_LABELLED_ELEMENT_CREATE(Button);
SCRIPT_EXT_LABELLED_ELEMENT_CREATE(Checkbox);
SCRIPT_EXT_LABELLED_ELEMENT_CREATE(Label);

#define SCRIPT_EXT_CSTRING_ELEMENT_CREATE(name) \
LIBRARY_EXPORT bool ScriptExt##name##Create(struct ExecutionContext *context) { \
	ElementWrapper *parent; \
	uint32_t flags; \
	char *cString; \
	bool success = ScriptParameterHandle(context, (void **) &parent) \
		&& ScriptParameterUint32(context, &flags) \
		&& ScriptParameterCString(context, &cString); \
	if (success) { \
		UI##name *element = UI##name##Create(parent ? parent->element : NULL, flags, cString); \
		ElementWrapper *wrapper = WrapperCreate(&element->e); \
		wrapper->cStringStore = cString; \
		success = ScriptReturnHandle(context, wrapper, ElementWrapperClose); \
	} else { \
		success = ScriptReturnHandle(context, NULL, NULL); \
		free(cString); \
	} \
	return success; \
}
SCRIPT_EXT_CSTRING_ELEMENT_CREATE(TabPane);
SCRIPT_EXT_CSTRING_ELEMENT_CREATE(Table);

#define SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(name) \
LIBRARY_EXPORT bool ScriptExt##name##Create(struct ExecutionContext *context) { \
	ElementWrapper *parent; \
	uint32_t flags; \
	bool success = ScriptParameterHandle(context, (void **) &parent) \
		&& ScriptParameterUint32(context, &flags); \
	if (success) { \
		UI##name *element = UI##name##Create(parent ? parent->element : NULL, flags); \
		return ScriptReturnHandle(context, WrapperCreate(&element->e), ElementWrapperClose); \
	} else { \
		return ScriptReturnHandle(context, NULL, NULL); \
	} \
}
SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(Code);
SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(Gauge);
SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(MDIClient);
SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(Panel);
SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(ScrollBar);
SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(Slider);
SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(Switcher);
SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(Textbox);
SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(WrapPanel);

LIBRARY_EXPORT bool ScriptExtSpacerCreate(struct ExecutionContext *context) {
	ElementWrapper *parent;
	uint32_t flags;
	int32_t width, height;
	return ScriptParameterScan(context, "huii", &parent, &flags, &width, &height)
		&& ScriptReturnHandle(context, WrapperCreate(&UISpacerCreate(parent ? parent->element : NULL, flags, width, height)->e), ElementWrapperClose);
}

LIBRARY_EXPORT bool ScriptExtSplitPaneCreate(struct ExecutionContext *context) {
	ElementWrapper *parent;
	uint32_t flags;
	double weight;
	return ScriptParameterScan(context, "huF", &parent, &flags, &weight)
		&& ScriptReturnHandle(context, WrapperCreate(&UISplitPaneCreate(parent ? parent->element : NULL, flags, weight)->e), ElementWrapperClose);
}

LIBRARY_EXPORT bool ScriptExtElementSetMessageUser(struct ExecutionContext *context) {
	ElementWrapper *element;
	intptr_t messageCallback = 0;

	bool success = ScriptParameterHandle(context, (void **) &element)
		&& ScriptParameterHeapRef(context, &messageCallback);

	if (success) {
		if (element->messageUser) {
			ScriptHeapRefClose(context, element->messageUser);
		}

		element->messageUser = messageCallback;
	} else {
		if (messageCallback) {
			ScriptHeapRefClose(context, messageCallback);
		}
	}

	return success;
}

LIBRARY_EXPORT bool ScriptExtElementGetClassName(struct ExecutionContext *context) {
	ElementWrapper *element;
	return ScriptParameterHandle(context, (void **) &element) && ReturnCString(context, element->element->cClassName);
}

LIBRARY_EXPORT bool ScriptExtElementGetWindow(struct ExecutionContext *context) {
	ElementWrapper *element;
	return ScriptParameterHandle(context, (void **) &element) && ScriptReturnHandle(context, WrapperCreate(&element->element->window->e), ElementWrapperClose);
}

LIBRARY_EXPORT bool ScriptExtElementGetParent(struct ExecutionContext *context) {
	ElementWrapper *element;
	return ScriptParameterHandle(context, (void **) &element) && ScriptReturnHandle(context, WrapperCreate(element->element->parent), ElementWrapperClose);
}

LIBRARY_EXPORT bool ScriptExtElementSetFlags(struct ExecutionContext *context) {
	ElementWrapper *element;
	uint32_t flags;
	if (!ScriptParameterScan(context, "hu", (void **) &element, &flags)) return false;
	element->element->flags = flags;
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementAnimate(struct ExecutionContext *context) {
	ElementWrapper *element;
	bool stop;
	if (!ScriptParameterScan(context, "hb", (void **) &element, &stop)) return false;
	UIElementAnimate(element->element, stop);
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementChangeParent(struct ExecutionContext *context) {
	ElementWrapper *element, *newParent, *insertBefore;
	return ScriptParameterScan(context, "hhh", &element, &newParent, &insertBefore) && ScriptReturnHandle(context, WrapperCreate(
				UIElementChangeParent(element->element, newParent->element, insertBefore ? insertBefore->element : NULL)), ElementWrapperClose);
}

LIBRARY_EXPORT bool ScriptExtElementDestroy(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementDestroy(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementDestroyDescendents(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementDestroyDescendents(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementFocus(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementFocus(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementRefresh(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementRelayout(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementRelayout(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementMeasurementsChanged(struct ExecutionContext *context) {
	ElementWrapper *element;
	int32_t which;
	if (!ScriptParameterScan(context, "hi", &element, &which)) return false;
	UIElementMeasurementsChanged(element->element, which);
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementFindByPoint(struct ExecutionContext *context) {
	ElementWrapper *element;
	int32_t x, y;
	return ScriptParameterScan(context, "hii", &element, &x, &y)
		&& ScriptReturnHandle(context, WrapperCreate(UIElementFindByPoint(element->element, x, y)), ElementWrapperClose);
}

LIBRARY_EXPORT bool ScriptExtElementSetContextUser(struct ExecutionContext *context) {
	ElementWrapper *element;
	intptr_t cp;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	if (!ScriptParameterHeapRef(context, &cp)) return false;
	if (element->contextUser) ScriptHeapRefClose(context, element->contextUser);
	element->contextUser = cp;
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementSetContextClass(struct ExecutionContext *context) {
	ElementWrapper *element;
	intptr_t cp;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	if (!ScriptParameterHeapRef(context, &cp)) return false;
	if (element->contextClass) ScriptHeapRefClose(context, element->contextClass);
	element->contextClass = cp;
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementGetContextUser(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ScriptReturnHeapRef(context, element->contextUser);
}

LIBRARY_EXPORT bool ScriptExtElementGetContextClass(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ScriptReturnHeapRef(context, element->contextClass);
}

LIBRARY_EXPORT bool ScriptExtElementScreenBounds(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ReturnRectangle(context, UIElementScreenBounds(element->element));
}

LIBRARY_EXPORT bool ScriptExtElementWindowBounds(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ReturnRectangle(context, element->element->bounds);
}

LIBRARY_EXPORT bool ScriptExtElementGetClip(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ReturnRectangle(context, element->element->clip);
}

LIBRARY_EXPORT bool ScriptExtElementMove(struct ExecutionContext *context) {
	ElementWrapper *element;
	UIRectangle rect;
	bool alwaysLayout;
	if (!ScriptParameterScan(context, "h(iiii)b", (void **) &element, &rect.l, &rect.r, &rect.t, &rect.b, &alwaysLayout)) return false;
	UIElementMove(element->element, rect, alwaysLayout);
	return true;
}

LIBRARY_EXPORT bool ScriptExtElementRepaint(struct ExecutionContext *context) {
	ElementWrapper *element;
	UIRectangle rect;
	bool isRectNull;
	if (!ScriptParameterScan(context, "h(niiii)", (void **) &element, &isRectNull, &rect.l, &rect.r, &rect.t, &rect.b)) return false;
	UIElementRepaint(element->element, isRectNull ? NULL : &rect);
	return true;
}

LIBRARY_EXPORT bool ScriptExtButtonSetLabel(struct ExecutionContext *context) {
	ElementWrapper *element;
	const char *label; size_t labelBytes;
	if (!ScriptParameterScan(context, "hS", (void **) &element, &label, &labelBytes)) return false;
	UIButtonSetLabel((UIButton *) element->element, label, labelBytes);
	return true;
}

LIBRARY_EXPORT bool ScriptExtLabelSetContent(struct ExecutionContext *context) {
	ElementWrapper *element;
	const char *label; size_t labelBytes;
	if (!ScriptParameterScan(context, "hS", (void **) &element, &label, &labelBytes)) return false;
	UILabelSetContent((UILabel *) element->element, label, labelBytes);
	return true;
}

LIBRARY_EXPORT bool ScriptExtPanelSetSpacing(struct ExecutionContext *context) {
	ElementWrapper *element;
	UIRectangle border; int gap;
	if (!ScriptParameterScan(context, "h(iiii)i", (void **) &element, &border.l, &border.r, &border.t, &border.b, &gap)) return false;
	UIPanel *panel = (UIPanel *) element->element;
	panel->border = border;
	panel->gap = gap;
	UIElementMeasurementsChanged(element->element, 3);
	return true;
}

LIBRARY_EXPORT bool ScriptExtGaugeSetPosition(struct ExecutionContext *context) {
	ElementWrapper *element;
	double value;
	if (!ScriptParameterScan(context, "hF", (void **) &element, &value)) return false;
	UIGaugeSetPosition((UIGauge *) element->element, value);
	return true;
}

LIBRARY_EXPORT bool ScriptExtTextboxReplace(struct ExecutionContext *context) {
	ElementWrapper *element;
	const char *text; size_t textBytes; bool sendChangedMessage;
	if (!ScriptParameterScan(context, "hSb", (void **) &element, &text, &textBytes, &sendChangedMessage)) return false;
	UITextboxReplace((UITextbox *) element->element, text, textBytes, sendChangedMessage);
	return true;
}

LIBRARY_EXPORT bool ScriptExtTextboxClear(struct ExecutionContext *context) {
	ElementWrapper *element;
	bool sendChangedMessage;
	if (!ScriptParameterScan(context, "hb", (void **) &element, &sendChangedMessage)) return false;
	UITextboxClear((UITextbox *) element->element, sendChangedMessage);
	return true;
}

LIBRARY_EXPORT bool ScriptExtTextboxMoveCaret(struct ExecutionContext *context) {
	ElementWrapper *element;
	bool backward, word;
	if (!ScriptParameterScan(context, "hbb", (void **) &element, &backward, &word)) return false;
	UITextboxMoveCaret((UITextbox *) element->element, backward, word);
	return true;
}

LIBRARY_EXPORT bool ScriptExtTextboxGetContents(struct ExecutionContext *context) {
	ElementWrapper *element;
	return ScriptParameterScan(context, "h", (void **) &element)
		&& ScriptReturnString(context, ((UITextbox *) element->element)->string, ((UITextbox *) element->element)->bytes);
}

LIBRARY_EXPORT bool ScriptExtTextboxRejectNextKey(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterScan(context, "h", (void **) &element)) return false;
	((UITextbox *) element->element)->rejectNextKey = true;
	return true;
}

LIBRARY_EXPORT bool ScriptExtTextboxSetSelectionFrom(struct ExecutionContext *context) {
	ElementWrapper *element;
	int32_t index;
	if (!ScriptParameterScan(context, "hi", (void **) &element, &index)) return false;
	((UITextbox *) element->element)->carets[0] = index;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtTextboxSetSelectionTo(struct ExecutionContext *context) {
	ElementWrapper *element;
	int32_t index;
	if (!ScriptParameterScan(context, "hi", (void **) &element, &index)) return false;
	((UITextbox *) element->element)->carets[1] = index;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtTextboxSetCaretPosition(struct ExecutionContext *context) {
	ElementWrapper *element;
	int32_t index;
	if (!ScriptParameterScan(context, "hi", (void **) &element, &index)) return false;
	((UITextbox *) element->element)->carets[0] = index;
	((UITextbox *) element->element)->carets[1] = index;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtSliderSetSteps(struct ExecutionContext *context) {
	ElementWrapper *element;
	int32_t steps;
	if (!ScriptParameterScan(context, "hi", (void **) &element, &steps)) return false;
	((UISlider *) element->element)->steps = steps;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtSliderSetPosition(struct ExecutionContext *context) {
	ElementWrapper *element;
	double position; bool sendChangedMessage;
	if (!ScriptParameterScan(context, "hFb", (void **) &element, &position, &sendChangedMessage)) return false;
	UISliderSetPosition((UISlider *) element->element, position, sendChangedMessage);
	return true;
}

LIBRARY_EXPORT bool ScriptExtCheckboxSetCheck(struct ExecutionContext *context) {
	ElementWrapper *element;
	int check;
	if (!ScriptParameterScan(context, "hi", (void **) &element, &check)) return false;
	((UICheckbox *) element->element)->check = check;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtKeycodeBackspace(struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_BACKSPACE); }
LIBRARY_EXPORT bool ScriptExtKeycodeDelete   (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_DELETE   ); }
LIBRARY_EXPORT bool ScriptExtKeycodeDown     (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_DOWN     ); }
LIBRARY_EXPORT bool ScriptExtKeycodeEnd      (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_END      ); }
LIBRARY_EXPORT bool ScriptExtKeycodeEnter    (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_ENTER    ); }
LIBRARY_EXPORT bool ScriptExtKeycodeEscape   (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_ESCAPE   ); }
LIBRARY_EXPORT bool ScriptExtKeycodeHome     (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_HOME     ); }
LIBRARY_EXPORT bool ScriptExtKeycodeInsert   (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_INSERT   ); }
LIBRARY_EXPORT bool ScriptExtKeycodeLeft     (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_LEFT     ); }
LIBRARY_EXPORT bool ScriptExtKeycodeRight    (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_RIGHT    ); }
LIBRARY_EXPORT bool ScriptExtKeycodeSpace    (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_SPACE    ); }
LIBRARY_EXPORT bool ScriptExtKeycodeTab      (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_TAB      ); }
LIBRARY_EXPORT bool ScriptExtKeycodeUp       (struct ExecutionContext *context) { return ScriptReturnInt(context, UI_KEYCODE_UP       ); }

LIBRARY_EXPORT bool ScriptExtKeycodeFKey(struct ExecutionContext *context) {
	int32_t n;
	return ScriptParameterInt32(context, &n) && ScriptReturnInt(context, UI_KEYCODE_FKEY(n)); 
}

LIBRARY_EXPORT bool ScriptExtKeycodeDigit(struct ExecutionContext *context) {
	const char *s; size_t b;
	return ScriptParameterString(context, (const void **) &s, &b) && ScriptReturnInt(context, b ? UI_KEYCODE_DIGIT(s[0]) : 0); 
}

LIBRARY_EXPORT bool ScriptExtKeycodeLetter(struct ExecutionContext *context) {
	const char *s; size_t b;
	return ScriptParameterString(context, (const void **) &s, &b) && ScriptReturnInt(context, b ? UI_KEYCODE_LETTER(s[0]) : 0); 
}

LIBRARY_EXPORT bool ScriptExtDrawBlock(struct ExecutionContext *context) {
	PainterWrapper *painter; UIRectangle rectangle; uint32_t color;
	if (!ScriptParameterScan(context, "h(iiii)u", &painter, &rectangle.l, &rectangle.r, &rectangle.t, &rectangle.b, &color)) return false;
	if (!painter->painter) { return true; } UIDrawBlock(painter->painter, rectangle, color); return true;
}

LIBRARY_EXPORT bool ScriptExtDrawCircle(struct ExecutionContext *context) {
	PainterWrapper *painter; int32_t centerX, centerY; uint32_t radius, fillColor, outlineColor; bool hollow;
	if (!ScriptParameterScan(context, "hiiuuub", &painter, &centerX, &centerY, &radius, &fillColor, &outlineColor, &hollow)) return false;
	if (!painter->painter) { return true; } UIDrawCircle(painter->painter, centerX, centerY, radius, fillColor, outlineColor, hollow); return true;
}

LIBRARY_EXPORT bool ScriptExtDrawInvert(struct ExecutionContext *context) {
	PainterWrapper *painter; UIRectangle rectangle;
	if (!ScriptParameterScan(context, "h(iiii)", &painter, &rectangle.l, &rectangle.r, &rectangle.t, &rectangle.b)) return false;
	if (!painter->painter) { return true; } UIDrawInvert(painter->painter, rectangle); return true;
}

LIBRARY_EXPORT bool ScriptExtDrawLine(struct ExecutionContext *context) {
	PainterWrapper *painter; int32_t x0, y0, x1, y1; uint32_t color;
	if (!ScriptParameterScan(context, "hiiiiu", &painter, &x0, &y0, &x1, &y1, &color)) return false;
	return ScriptReturnInt(context, painter->painter && UIDrawLine(painter->painter, x0, y0, x1, y1, color));
}

LIBRARY_EXPORT bool ScriptExtDrawTriangle(struct ExecutionContext *context) {
	PainterWrapper *painter; int32_t x0, y0, x1, y1, x2, y2; uint32_t color;
	if (!ScriptParameterScan(context, "hiiiiu", &painter, &x0, &y0, &x1, &y1, &x2, &y2, &color)) return false;
	if (!painter->painter) { return true; } UIDrawTriangle(painter->painter, x0, y0, x1, y1, x2, y2, color); return true;
}

LIBRARY_EXPORT bool ScriptExtDrawTriangleOutline(struct ExecutionContext *context) {
	PainterWrapper *painter; int32_t x0, y0, x1, y1, x2, y2; uint32_t color;
	if (!ScriptParameterScan(context, "hiiiiiiu", &painter, &x0, &y0, &x1, &y1, &x2, &y2, &color)) return false;
	if (!painter->painter) { return true; } UIDrawTriangleOutline(painter->painter, x0, y0, x1, y1, x2, y2, color); return true;
}

LIBRARY_EXPORT bool ScriptExtDrawGlyph(struct ExecutionContext *context) {
	PainterWrapper *painter; int32_t x, y, c; uint32_t color;
	if (!ScriptParameterScan(context, "hiiiu", &painter, &x, &y, &c, &color)) return false;
	if (!painter->painter) { return true; } UIDrawGlyph(painter->painter, x, y, c, color); return true;
}

LIBRARY_EXPORT bool ScriptExtDrawRectangle(struct ExecutionContext *context) {
	PainterWrapper *painter; UIRectangle rectangle, borderSize; uint32_t mainColor, borderColor;
	if (!ScriptParameterScan(context, "h(iiii)uu(iiii)", &painter, &rectangle.l, &rectangle.r, &rectangle.t, &rectangle.b, &mainColor, &borderColor, &borderSize.l, &borderSize.r, &borderSize.t, &borderSize.b)) return false;
	if (!painter->painter) { return true; } UIDrawRectangle(painter->painter, rectangle, mainColor, borderColor, borderSize); return true;
}

LIBRARY_EXPORT bool ScriptExtDrawBorder(struct ExecutionContext *context) {
	PainterWrapper *painter; UIRectangle rectangle, borderSize; uint32_t borderColor;
	if (!ScriptParameterScan(context, "h(iiii)u(iiii)", &painter, &rectangle.l, &rectangle.r, &rectangle.t, &rectangle.b, &borderColor, &borderSize.l, &borderSize.r, &borderSize.t, &borderSize.b)) return false;
	if (!painter->painter) { return true; } UIDrawBorder(painter->painter, rectangle, borderColor, borderSize); return true;
}

LIBRARY_EXPORT bool ScriptExtDrawString(struct ExecutionContext *context) {
	PainterWrapper *painter; UIRectangle rectangle; const char *string; size_t stringBytes; 
	uint32_t color; int32_t align; UIStringSelection selection; bool selectionIsNull;
	if (!ScriptParameterScan(context, "h(iiii)Sui(niiuu)", &painter, &rectangle.l, &rectangle.r, &rectangle.t, &rectangle.b, 
				&string, &stringBytes, &color, &align, &selectionIsNull, &selection.carets[0], 
				&selection.carets[1], &selection.colorText, &selection.colorBackground)) return false;
	if (!painter->painter) { return true; } 
	UIDrawString(painter->painter, rectangle, string, stringBytes, color, align, selectionIsNull ? NULL : &selection); return true;
}

LIBRARY_EXPORT bool ScriptExtDrawStringHighlighted(struct ExecutionContext *context) {
	PainterWrapper *painter; UIRectangle rectangle; const char *string; size_t stringBytes; int32_t tabSize;
	if (!ScriptParameterScan(context, "h(iiii)Si", &painter, &rectangle.l, &rectangle.r, &rectangle.t, &rectangle.b, &string, &stringBytes, &tabSize)) return false;
	if (!painter->painter) { return true; } 
	int finalX = UIDrawStringHighlighted(painter->painter, rectangle, string, stringBytes, tabSize); 
	return ScriptReturnInt(context, finalX);
}

LIBRARY_EXPORT bool ScriptExtPainterGetWidth(struct ExecutionContext *context) {
	PainterWrapper *painter; if (!ScriptParameterScan(context, "h", &painter)) return false;
	return ScriptReturnInt(context, painter->painter ? painter->painter->width : -1);
}

LIBRARY_EXPORT bool ScriptExtPainterGetHeight(struct ExecutionContext *context) {
	PainterWrapper *painter; if (!ScriptParameterScan(context, "h", &painter)) return false;
	return ScriptReturnInt(context, painter->painter ? painter->painter->height : -1);
}

LIBRARY_EXPORT bool ScriptExtPainterGetClip(struct ExecutionContext *context) {
	UIRectangle invalid = UI_RECT_1(-1);
	PainterWrapper *painter; if (!ScriptParameterScan(context, "h", &painter)) return false;
	return ReturnRectangle(context, painter->painter ? painter->painter->clip : invalid);
}

LIBRARY_EXPORT bool ScriptExtMeasureStringWidth(struct ExecutionContext *context) {
	const char *string; size_t stringBytes;
	if (!ScriptParameterScan(context, "S", &string, &stringBytes)) return false;
	return ScriptReturnInt(context, UIMeasureStringWidth(string, stringBytes));
}

LIBRARY_EXPORT bool ScriptExtMeasureStringHeight(struct ExecutionContext *context) {
	return ScriptReturnInt(context, UIMeasureStringHeight());
}

LIBRARY_EXPORT bool ScriptExtAnimateClock(struct ExecutionContext *context) {
	return ScriptReturnInt(context, UIAnimateClock());
}

LIBRARY_EXPORT bool ScriptExtWindowPack(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t width;
	if (!ScriptParameterScan(context, "hi", &element, &width)) return false;
	UIWindowPack((UIWindow *) element->element, width);
	return true;
}

LIBRARY_EXPORT bool ScriptExtWindowSetTextboxModifiedFlag(struct ExecutionContext *context) {
	ElementWrapper *element; bool value;
	if (!ScriptParameterScan(context, "hb", &element, &value)) return false;
	((UIWindow *) element->element)->textboxModifiedFlag = value;
	return true;
}

LIBRARY_EXPORT bool ScriptExtTableSetHighlightedColumn(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t index;
	if (!ScriptParameterScan(context, "hi", &element, &index)) return false;
	((UITable *) element->element)->columnHighlight = index;
	UIElementRepaint(element->element, NULL);
	return true;
}

LIBRARY_EXPORT bool ScriptExtTableSetItemCount(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t index;
	if (!ScriptParameterScan(context, "hi", &element, &index)) return false;
	((UITable *) element->element)->itemCount = index;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtTableHitTest(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t x, y;
	if (!ScriptParameterScan(context, "hii", &element, &x, &y)) return false;
	return ScriptReturnInt(context, UITableHitTest((UITable *) element->element, x, y));
}

LIBRARY_EXPORT bool ScriptExtTableHeaderHitTest(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t x, y;
	if (!ScriptParameterScan(context, "hii", &element, &x, &y)) return false;
	return ScriptReturnInt(context, UITableHeaderHitTest((UITable *) element->element, x, y));
}

LIBRARY_EXPORT bool ScriptExtTableEnsureVisible(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t index;
	if (!ScriptParameterScan(context, "hi", &element, &index)) return false;
	return ScriptReturnInt(context, UITableEnsureVisible((UITable *) element->element, index));
}

LIBRARY_EXPORT bool ScriptExtTableResizeColumns(struct ExecutionContext *context) {
	bool alreadyHasContext = contextForCallback != NULL;
	if (!alreadyHasContext) contextForCallback = context;
	ElementWrapper *element;
	if (!ScriptParameterScan(context, "h", &element)) return false;
	UITableResizeColumns((UITable *) element->element);
	if (!alreadyHasContext) contextForCallback = NULL;
	return true;
}

LIBRARY_EXPORT bool ScriptExtTabPaneSetActive(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t index;
	if (!ScriptParameterScan(context, "hi", &element, &index)) return false;
	((UITabPane *) element->element)->active = index;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtTabPaneGetActive(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterScan(context, "h", &element)) return false;
	return ScriptReturnInt(context, ((UITabPane *) element->element)->active);
}

LIBRARY_EXPORT bool ScriptExtScrollBarSetMaximumPosition(struct ExecutionContext *context) {
	ElementWrapper *element; int64_t value;
	if (!ScriptParameterScan(context, "hI", &element, &value)) return false;
	((UIScrollBar *) element->element)->maximum = value;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtScrollBarSetPageSize(struct ExecutionContext *context) {
	ElementWrapper *element; int64_t value;
	if (!ScriptParameterScan(context, "hI", &element, &value)) return false;
	((UIScrollBar *) element->element)->page = value;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtScrollBarSetPosition(struct ExecutionContext *context) {
	ElementWrapper *element; double value;
	if (!ScriptParameterScan(context, "hF", &element, &value)) return false;
	((UIScrollBar *) element->element)->position = value;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtScrollBarGetPosition(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterScan(context, "h", &element)) return false;
	return ScriptReturnDouble(context, ((UIScrollBar *) element->element)->position);
}

LIBRARY_EXPORT bool ScriptExtMDIChildCreate(struct ExecutionContext *context) {
	ElementWrapper *parent; uint32_t flags; UIRectangle bounds; const char *title; size_t titleBytes;
	if (!ScriptParameterScan(context, "hi(iiii)S", &parent, &flags, &bounds.l, &bounds.r, &bounds.t, &bounds.b, &title, &titleBytes)) return false;
	UIMDIChild *element = UIMDIChildCreate(parent ? parent->element : NULL, flags, bounds, title, titleBytes);
	return ScriptReturnHandle(context, WrapperCreate(&element->e), ElementWrapperClose);
}

LIBRARY_EXPORT bool ScriptExtMDIChildGetBounds(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterScan(context, "h", &element)) return false;
	return ReturnRectangle(context, ((UIMDIChild *) element->element)->bounds);
}

LIBRARY_EXPORT bool ScriptExtMDIChildSetBounds(struct ExecutionContext *context) {
	ElementWrapper *element; UIRectangle bounds;
	if (!ScriptParameterScan(context, "h(iiii)", &element, &bounds.l, &bounds.r, &bounds.t, &bounds.b)) return false;
	((UIMDIChild *) element->element)->bounds = bounds;
	UIElementRefresh(element->element);
	UIElementRefresh(element->element->parent);
	return true;
}

LIBRARY_EXPORT bool ScriptExtCodeFocusLine(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t index;
	if (!ScriptParameterScan(context, "hi", &element, &index)) return false;
	UICodeFocusLine((UICode *) element->element, index);
	return true;
}

LIBRARY_EXPORT bool ScriptExtCodeHitTest(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t x, y;
	if (!ScriptParameterScan(context, "hii", &element, &x, &y)) return false;
	return ScriptReturnInt(context, UICodeHitTest((UICode *) element->element, x, y));
}

LIBRARY_EXPORT bool ScriptExtCodeInsertContent(struct ExecutionContext *context) {
	ElementWrapper *element; const char *content; size_t contentBytes; bool replace;
	if (!ScriptParameterScan(context, "hSb", &element, &content, &contentBytes, &replace)) return false;
	UICodeInsertContent((UICode *) element->element, content, contentBytes, replace);
	return true;
}

LIBRARY_EXPORT bool ScriptExtCodeSetTabSize(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t tabSize;
	if (!ScriptParameterScan(context, "hi", &element, &tabSize)) return false;
	((UICode *) element->element)->tabSize = tabSize;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtCodeGetLineCount(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterScan(context, "h", &element)) return false;
	return ScriptReturnInt(context, ((UICode *) element->element)->lineCount);
}

LIBRARY_EXPORT bool ScriptExtCodeGetLineContent(struct ExecutionContext *context) {
	ElementWrapper *element; int32_t index;
	if (!ScriptParameterScan(context, "hi", &element, &index)) return false;
	UICode *code = (UICode *) element->element;

	if (index >= 1 && index <= code->lineCount) {
		return ScriptReturnString(context, code->lines[index].offset + code->content, code->lines[index].bytes);
	} else {
		return ScriptReturnString(context, NULL, 0);
	}
}

LIBRARY_EXPORT bool ScriptExtSplitPaneGetWeight(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterScan(context, "h", &element)) return false;
	return ScriptReturnDouble(context, ((UISplitPane *) element->element)->weight);
}

LIBRARY_EXPORT bool ScriptExtSplitPaneSetWeight(struct ExecutionContext *context) {
	ElementWrapper *element; double weight;
	if (!ScriptParameterScan(context, "hF", &element, &weight)) return false;
	((UISplitPane *) element->element)->weight = weight;
	UIElementRefresh(element->element);
	return true;
}

LIBRARY_EXPORT bool ScriptExtImageDisplayCreate(struct ExecutionContext *context) {
	ElementWrapper *parent; uint32_t flags; UIPainter *bitmap;
	if (!ScriptParameterScan(context, "hih", &parent, &flags, &bitmap)) return false;
	UIImageDisplay *element = UIImageDisplayCreate(parent ? parent->element : NULL, flags, bitmap ? bitmap->bits : NULL, bitmap ? bitmap->width : 0, bitmap ? bitmap->height : 0, bitmap ? bitmap->width * 4 : 0);
	return ScriptReturnHandle(context, WrapperCreate(&element->e), ElementWrapperClose);
}

LIBRARY_EXPORT bool ScriptExtImageDisplaySetContent(struct ExecutionContext *context) {
	ElementWrapper *element; UIPainter *bitmap;
	if (!ScriptParameterScan(context, "hh", &element, &bitmap)) return false;
	UIImageDisplaySetContent((UIImageDisplay *) element->element, bitmap ? bitmap->bits : NULL, bitmap ? bitmap->width : 0, bitmap ? bitmap->height : 0, bitmap ? bitmap->width * 4 : 0);
	return true;
}
