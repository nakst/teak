#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define UI_IMPLEMENTATION
#include "luigi2.h"

#include "../native_interface.h"

typedef struct ElementWrapper {
	UIElement *element;
	uintptr_t referenceCount;
	intptr_t messageUser;
	intptr_t cp;
} ElementWrapper;

// TODO How should this be done properly?
struct ExecutionContext *contextForCallback;

void WrapperClose(struct ExecutionContext *context, void *_wrapper) {
	ElementWrapper *wrapper = (ElementWrapper *) _wrapper;

	wrapper->referenceCount--;

	if (wrapper->referenceCount == 0) {
		ScriptHeapRefClose(context, wrapper->messageUser);
		ScriptHeapRefClose(context, wrapper->cp);
		UI_ASSERT(!wrapper->element);
		free(wrapper);
	}
}

bool WrapperOpen(struct ExecutionContext *context, ElementWrapper *wrapper, intptr_t *handle) {
	UI_ASSERT(wrapper->referenceCount);
	wrapper->referenceCount++;
	return ScriptCreateHandle(context, wrapper, WrapperClose, handle);
}

int WrapperMessage(UIElement *element, UIMessage message, int di, void *dp) {
	ElementWrapper *wrapper = (ElementWrapper *) element->cp;

	if (!wrapper) {
		return 0;
	}

	if (wrapper->messageUser) {
		intptr_t elementHandle = -1;
		bool success = WrapperOpen(contextForCallback, wrapper, &elementHandle);
		(void) success;
		// TODO How to handle failure?

		int64_t parameters[4] = {
			(int64_t) elementHandle,
			(int64_t) message,
			(int64_t) di,
			0, // TODO MessageData.
		};

		bool managedParameters[4] = { true, false, false, true };
		// TODO Modify this so that it actually works!
		success = ScriptRunCallback(contextForCallback, wrapper->messageUser, parameters, managedParameters, 4);
		// TODO How to handle failure?

		ScriptHeapRefClose(contextForCallback, elementHandle);
	}

	if (message == UI_MSG_DESTROY) {
		WrapperClose(contextForCallback, wrapper);
		element->cp = NULL;
		wrapper->element = NULL;
	}

	return 0;
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

bool ScriptExtInitialise(struct ExecutionContext *context) {
	UIInitialise();
	return true;
}

bool ScriptExtMessageLoop(struct ExecutionContext *context) {
	contextForCallback = context;
	ScriptReturnInt(context, UIMessageLoop());
	return true;
}

bool ScriptExtWindowCreate(struct ExecutionContext *context) {
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
		ScriptReturnHandle(context, WrapperCreate(&window->e), WrapperClose);
	} else {
		ScriptReturnHandle(context, NULL, NULL);
	}

	free(title);
	return success;
}

#define SCRIPT_EXT_LABELLED_ELEMENT_CREATE(name) \
bool ScriptExt##name##Create(struct ExecutionContext *context) { \
	ElementWrapper *parent; \
	uint32_t flags; \
	const char *string; \
	size_t stringBytes; \
	bool success = ScriptParameterHandle(context, (void **) &parent) \
		&& ScriptParameterUint32(context, &flags) \
		&& ScriptParameterString(context, (const void **) &string, &stringBytes); \
	if (success) { \
		UI##name *element = UI##name##Create(parent->element, flags, string, stringBytes); \
		ScriptReturnHandle(context, WrapperCreate(&element->e), WrapperClose); \
	} else { \
		ScriptReturnHandle(context, NULL, NULL); \
	} \
	return success; \
}
SCRIPT_EXT_LABELLED_ELEMENT_CREATE(Button);
SCRIPT_EXT_LABELLED_ELEMENT_CREATE(Checkbox);
SCRIPT_EXT_LABELLED_ELEMENT_CREATE(Label);

#define SCRIPT_EXT_CSTRING_ELEMENT_CREATE(name) \
bool ScriptExt##name##Create(struct ExecutionContext *context) { \
	ElementWrapper *parent; \
	uint32_t flags; \
	char *tabs; \
	bool success = ScriptParameterHandle(context, (void **) &parent) \
		&& ScriptParameterUint32(context, &flags) \
		&& ScriptParameterCString(context, &tabs); \
	if (success) { \
		UI##name *element = UI##name##Create(parent->element, flags, tabs); \
		ScriptReturnHandle(context, WrapperCreate(&element->e), WrapperClose); \
	} else { \
		ScriptReturnHandle(context, NULL, NULL); \
	} \
	free(tabs); \
	return success; \
}
SCRIPT_EXT_CSTRING_ELEMENT_CREATE(TabPane);
SCRIPT_EXT_CSTRING_ELEMENT_CREATE(Table);

#define SCRIPT_EXT_SIMPLE_ELEMENT_CREATE(name) \
bool ScriptExt##name##Create(struct ExecutionContext *context) { \
	ElementWrapper *parent; \
	uint32_t flags; \
	bool success = ScriptParameterHandle(context, (void **) &parent) \
		&& ScriptParameterUint32(context, &flags); \
	if (success) { \
		UI##name *element = UI##name##Create(parent->element, flags); \
		ScriptReturnHandle(context, WrapperCreate(&element->e), WrapperClose); \
	} else { \
		ScriptReturnHandle(context, NULL, NULL); \
	} \
	return success; \
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

bool ScriptExtSpacerCreate(struct ExecutionContext *context) {
	ElementWrapper *parent;
	uint32_t flags;
	int32_t width, height;

	bool success = ScriptParameterHandle(context, (void **) &parent)
		&& ScriptParameterUint32(context, &flags)
		&& ScriptParameterInt32(context, &width)
		&& ScriptParameterInt32(context, &height);

	if (success) {
		UISpacer *element = UISpacerCreate(parent->element, flags, width, height);
		ScriptReturnHandle(context, WrapperCreate(&element->e), WrapperClose);
	} else {
		ScriptReturnHandle(context, NULL, NULL);
	}

	return success;
}

bool ScriptExtSplitPaneCreate(struct ExecutionContext *context) {
	ElementWrapper *parent;
	uint32_t flags;
	double weight;

	bool success = ScriptParameterHandle(context, (void **) &parent)
		&& ScriptParameterUint32(context, &flags)
		&& ScriptParameterDouble(context, &weight);

	if (success) {
		UISplitPane *element = UISplitPaneCreate(parent->element, flags, weight);
		ScriptReturnHandle(context, WrapperCreate(&element->e), WrapperClose);
	} else {
		ScriptReturnHandle(context, NULL, NULL);
	}

	return success;
}

bool ScriptExtExpandPaneCreate(struct ExecutionContext *context) {
	ElementWrapper *parent;
	uint32_t flags, panelFlags;
	const char *label;
	size_t labelBytes;

	bool success = ScriptParameterHandle(context, (void **) &parent)
		&& ScriptParameterUint32(context, &flags)
		&& ScriptParameterString(context, (const void **) &label, &labelBytes)
		&& ScriptParameterUint32(context, &panelFlags);

	if (success) {
		UIExpandPane *element = UIExpandPaneCreate(parent->element, flags, label, labelBytes, panelFlags);
		ScriptReturnHandle(context, WrapperCreate(&element->e), WrapperClose);
	} else {
		ScriptReturnHandle(context, NULL, NULL);
	}

	return success;
}

bool ScriptExtElementSetMessageUser(struct ExecutionContext *context) {
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

bool ScriptExtElementGetClassName(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ScriptReturnString(context, element->element->cClassName, strlen(element->element->cClassName));
}

bool ScriptExtElementGetWindow(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ScriptReturnHandle(context, WrapperCreate(&element->element->window->e), WrapperClose);
}

bool ScriptExtElementGetParent(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ScriptReturnHandle(context, WrapperCreate(element->element->parent), WrapperClose);
}

bool ScriptExtElementGetFlags(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ScriptReturnInt(context, element->element->flags);
}

bool ScriptExtElementSetFlags(struct ExecutionContext *context) {
	ElementWrapper *element;
	uint32_t flags;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	if (!ScriptParameterUint32(context, &flags)) return false;
	element->element->flags = flags;
	return true;
}

bool ScriptExtElementAnimate(struct ExecutionContext *context) {
	ElementWrapper *element;
	bool stop;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	if (!ScriptParameterBool(context, &stop)) return false;
	UIElementAnimate(element->element, stop);
	return true;
}

bool ScriptExtElementChangeParent(struct ExecutionContext *context) {
	ElementWrapper *element;
	ElementWrapper *newParent;
	ElementWrapper *insertBefore;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	if (!ScriptParameterHandle(context, (void **) &newParent)) return false;
	if (!ScriptParameterHandle(context, (void **) &insertBefore)) return false;
	UIElement *result = UIElementChangeParent(element->element, newParent->element, insertBefore ? insertBefore->element : NULL);
	return ScriptReturnHandle(context, WrapperCreate(result), WrapperClose);
}

bool ScriptExtElementDestroy(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementDestroy(element->element);
	return true;
}

bool ScriptExtElementDestroyDescendents(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementDestroyDescendents(element->element);
	return true;
}

bool ScriptExtElementFocus(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementFocus(element->element);
	return true;
}

bool ScriptExtElementRefresh(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementRefresh(element->element);
	return true;
}

bool ScriptExtElementRelayout(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	UIElementRelayout(element->element);
	return true;
}

bool ScriptExtElementMeasurementsChanged(struct ExecutionContext *context) {
	ElementWrapper *element;
	int32_t which;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	if (!ScriptParameterInt32(context, &which)) return false;
	UIElementMeasurementsChanged(element->element, which);
	return true;
}

bool ScriptExtElementFindByPoint(struct ExecutionContext *context) {
	ElementWrapper *element;
	int32_t x, y;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	if (!ScriptParameterInt32(context, &x)) return false;
	if (!ScriptParameterInt32(context, &y)) return false;
	UIElement *result = UIElementFindByPoint(element->element, x, y);
	return ScriptReturnHandle(context, WrapperCreate(result), WrapperClose);
}

bool ScriptExtElementSetContext(struct ExecutionContext *context) {
	ElementWrapper *element;
	intptr_t cp;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	if (!ScriptParameterHeapRef(context, &cp)) return false;
	if (element->cp) ScriptHeapRefClose(context, element->cp);
	element->cp = cp;
	return true;
}

bool ScriptExtElementGetContext(struct ExecutionContext *context) {
	ElementWrapper *element;
	if (!ScriptParameterHandle(context, (void **) &element)) return false;
	return ScriptReturnHeapRef(context, element->cp);
}

bool ScriptExtRectangleContains(struct ExecutionContext *context) {
	bool success = false;
	intptr_t index = -1;
	int32_t x, y;
	UIRectangle rectangle;
	if (!ScriptParameterHeapRef(context, &index)) goto error;
	if (!ScriptParameterInt32(context, &x)) goto error;
	if (!ScriptParameterInt32(context, &y)) goto error;
	if (!ScriptStructReadInt32(context, index, 0, &rectangle.l)) goto error;
	if (!ScriptStructReadInt32(context, index, 1, &rectangle.r)) goto error;
	if (!ScriptStructReadInt32(context, index, 2, &rectangle.t)) goto error;
	if (!ScriptStructReadInt32(context, index, 3, &rectangle.b)) goto error;
	if (!ScriptReturnInt(context, UIRectangleContains(rectangle, x, y))) goto error;
	success = true;
	error:;
	if (index != -1) ScriptHeapRefClose(context, index);
	return success;
}

bool ScriptExtRectangleEquals(struct ExecutionContext *context) {
	bool success = false;
	intptr_t index1 = -1, index2 = -1;
	UIRectangle rectangle1, rectangle2;
	if (!ScriptParameterHeapRef(context, &index1)) goto error;
	if (!ScriptParameterHeapRef(context, &index2)) goto error;
	if (!ScriptStructReadInt32(context, index1, 0, &rectangle1.l)) goto error;
	if (!ScriptStructReadInt32(context, index1, 1, &rectangle1.r)) goto error;
	if (!ScriptStructReadInt32(context, index1, 2, &rectangle1.t)) goto error;
	if (!ScriptStructReadInt32(context, index1, 3, &rectangle1.b)) goto error;
	if (!ScriptStructReadInt32(context, index2, 0, &rectangle2.l)) goto error;
	if (!ScriptStructReadInt32(context, index2, 1, &rectangle2.r)) goto error;
	if (!ScriptStructReadInt32(context, index2, 2, &rectangle2.t)) goto error;
	if (!ScriptStructReadInt32(context, index2, 3, &rectangle2.b)) goto error;
	if (!ScriptReturnInt(context, UIRectangleEquals(rectangle1, rectangle2))) goto error;
	success = true;
	error:;
	if (index1 != -1) ScriptHeapRefClose(context, index1);
	if (index2 != -1) ScriptHeapRefClose(context, index2);
	return success;
}
