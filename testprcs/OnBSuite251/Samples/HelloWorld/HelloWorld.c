#define AppCreator 'Hell'
#define OptionsMenuAbout 1000
#define AboutAlert 1000
#define MainForm 1000
#define OK_BUTTON 1001
#define TEXT_FIELD 1000

char *DefaultText = "Hello World";

typedef struct {
	int skeletonData;
} Prefs;


static void *getObjectPtr(FormPtr frmP, Int16 index)
{
	UInt16 fldIndex = FrmGetObjectIndex(frmP, index);
	return FrmGetObjectPtr(frmP, fldIndex);
}

static void setDefaultText()
{
	FormPtr frmP = FrmGetActiveForm();
	FieldPtr fldP = getObjectPtr(frmP, TEXT_FIELD);
	MemHandle oldH = FldGetTextHandle(fldP);
	int length = StrLen(DefaultText) + 8;
	MemHandle h = MemHandleNew(length + 1);
	StrCopy(MemHandleLock(h),  DefaultText);
	MemHandleUnlock(h);
	FldSetText(fldP, h, 0, length + 1);
	if (oldH != NULL) 
		MemHandleFree(oldH);
}

static void mainFormInit(FormPtr frmP)
{
	FormPtr frmP = FrmGetActiveForm();
	setDefaultText();
	FrmDrawForm(frmP);
}

static Boolean doMenu(FormPtr frmP, UInt16 command)
{
	Boolean handled = false;
	switch (command) {
		case OptionsMenuAbout :
			FrmAlert(AboutAlert);
			handled = true;
			break;
	}
	return handled;
}

void switchText()
{
	FormPtr frmP = FrmGetActiveForm();
	FieldPtr fldP = getObjectPtr(frmP, TEXT_FIELD);
	MemHandle h = FldGetTextHandle(fldP);
	if (h != NULL) {
		char *p = MemHandleLock(h);
		int length = StrLen(p);
		if (length == 0) {
			MemHandleUnlock(h);
			setDefaultText();
		}
		else {
			int i;
			for (i = 0; i < length/2; i++) {
				char t = p[length - 1 - i];
				p[length - 1 - i] = p[i];
				p[i] = t;
			}
			MemHandleUnlock(h);
			FldSetText(fldP, h, 0, length + 1);
		}
		FrmDrawForm(frmP);
	}
}

Boolean mainFormEventHandler(EventPtr eventP)
{
	Boolean handled = false;
	FormPtr frmP = FrmGetActiveForm();
	switch (eventP->eType) {
  		case frmOpenEvent:
  			FrmDrawForm(frmP);
  			mainFormInit(frmP);
  			handled = true;
			break; 
		case menuEvent:
			handled = doMenu(frmP, eventP->data.menu.itemID);
			break;
		case ctlSelectEvent : {
			switch (eventP->data.ctlSelect.controlID) {
				case OK_BUTTON :
					switchText();
					handled = true;
					break;
			}
		}
		break;
	}
	return handled;
}

void startApp()
{
	Prefs prefs = { 0 };
	UInt16 prefSize = sizeof(Prefs);
	if ((PrefGetAppPreferences(AppCreator, 1000, &prefs, &prefSize, false) == noPreferenceFound) 
			|| (prefSize != sizeof(Prefs))) {
		// default initialization, since discovered Prefs was missing or old.
	}
}

void stopApp()
{
	Prefs prefs = { 0 };
	PrefSetAppPreferences(AppCreator, 1000, 1, &prefs, sizeof(Prefs), false);
}


Boolean appHandleEvent(EventPtr event)
{
	FormPtr	frm;
	Int16		formId;
	Boolean	handled = false;
	if (event->eType == frmLoadEvent) {
		formId = event->data.frmLoad.formID;
		frm = FrmInitForm(formId);
		FrmSetActiveForm(frm);
		if (formId == MainForm)
			FrmSetEventHandler (frm, mainFormEventHandler);
		handled = true;
	}	
	return handled;
}

UInt32 PilotMain(UInt16 cmd, char *cmdPBP, UInt16 launchFlags)
{
	EventType event;
	UInt16 error;
	if (cmd == sysAppLaunchCmdNormalLaunch) {
		startApp();
		FrmGotoForm(MainForm);
		do {
			EvtGetEvent(&event, evtWaitForever);
			if (!SysHandleEvent(&event))
				if (!MenuHandleEvent(0, &event, &error))
    					if (!appHandleEvent(&event))
						FrmDispatchEvent(&event);
		} while (event.eType != appStopEvent);
		stopApp();
		FrmCloseAllForms();
	}
	return 0;
}
 
