/*
*****************************************************************
                          COPYRIGHT NOTIFICATION
*****************************************************************

THE FOLLOWING IS A NOTICE OF COPYRIGHT, AVAILABILITY OF THE CODE,
AND DISCLAIMER WHICH MUST BE INCLUDED IN THE PROLOGUE OF THE CODE
AND IN ALL SOURCE LISTINGS OF THE CODE.

(C)  COPYRIGHT 1993 UNIVERSITY OF CHICAGO

Argonne National Laboratory (ANL), with facilities in the States of
Illinois and Idaho, is owned by the United States Government, and
operated by the University of Chicago under provision of a contract
with the Department of Energy.

Portions of this material resulted from work developed under a U.S.
Government contract and are subject to the following license:  For
a period of five years from March 30, 1993, the Government is
granted for itself and others acting on its behalf a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, and perform
publicly and display publicly.  With the approval of DOE, this
period may be renewed for two additional five year periods.
Following the expiration of this period or periods, the Government
is granted for itself and others acting on its behalf, a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, distribute copies
to the public, perform publicly and display publicly, and to permit
others to do so.

*****************************************************************
                                DISCLAIMER
*****************************************************************

NEITHER THE UNITED STATES GOVERNMENT NOR ANY AGENCY THEREOF, NOR
THE UNIVERSITY OF CHICAGO, NOR ANY OF THEIR EMPLOYEES OR OFFICERS,
MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL
LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR
USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY
OWNED RIGHTS.

*****************************************************************
LICENSING INQUIRIES MAY BE DIRECTED TO THE INDUSTRIAL TECHNOLOGY
DEVELOPMENT CENTER AT ARGONNE NATIONAL LABORATORY (630-252-2000).
*/
/*****************************************************************************
 *
 *     Original Author : Mark Anderson
 *     Second Author   : Frederick Vong
 *     Third Author    : Kenneth Evans, Jr.
 *
 *****************************************************************************
*/

#include "medm.h"

typedef struct _MedmIndicator {
    DlElement   *dlElement;     /* Must be first */
    Record      *record;
    UpdateTask  *updateTask;
} MedmIndicator;

static void indicatorUpdateValueCb(XtPointer cd);
static void indicatorDraw(XtPointer cd);
static void indicatorUpdateGraphicalInfoCb(XtPointer cd);
static void indicatorDestroyCb(XtPointer cd);
static void indicatorGetRecord(XtPointer, Record **, int *);
static void indicatorInheritValues(ResourceBundle *pRCB, DlElement *p);
static void indicatorSetBackgroundColor(ResourceBundle *pRCB, DlElement *p);
static void indicatorSetForegroundColor(ResourceBundle *pRCB, DlElement *p);
static void indicatorGetLimits(DlElement *pE, DlLimits **ppL, char **pN);
static void indicatorGetValues(ResourceBundle *pRCB, DlElement *p);

static DlDispatchTable indicatorDlDispatchTable = {
    createDlIndicator,
    NULL,
    executeDlIndicator,
    hideDlIndicator,
    writeDlIndicator,
    indicatorGetLimits,
    indicatorGetValues,
    indicatorInheritValues,
    indicatorSetBackgroundColor,
    indicatorSetForegroundColor,
    genericMove,
    genericScale,
    genericOrient,
    NULL,
    NULL};

void executeDlIndicator(DisplayInfo *displayInfo, DlElement *dlElement)
{
    Arg args[33];
    int n;
    int usedHeight, usedCharWidth, bestSize, preferredHeight;
    Widget localWidget;
    MedmIndicator *pi;
    DlIndicator *dlIndicator = dlElement->structure.indicator;

  /* Don't do anyting if the element is hidden */
    if(dlElement->hidden) return;

    if (!dlElement->widget) {
	if (displayInfo->traversalMode == DL_EXECUTE) {
	    if(dlElement->data) {
		pi = (MedmIndicator *)dlElement->data;
	    } else {
		pi = (MedmIndicator *)malloc(sizeof(MedmIndicator));
		dlElement->data = (void *)pi;
		pi->dlElement = dlElement;
		pi->updateTask = updateTaskAddTask(displayInfo,
		  &(dlIndicator->object),
		  indicatorDraw,
		  (XtPointer)pi);
		
		if (pi->updateTask == NULL) {
		    medmPrintf(1,"\nexecuteDlIndicator: Memory allocation error\n");
		} else {
		    updateTaskAddDestroyCb(pi->updateTask,indicatorDestroyCb);
		    updateTaskAddNameCb(pi->updateTask,indicatorGetRecord);
		}
		pi->record = medmAllocateRecord(dlIndicator->monitor.rdbk,
		  indicatorUpdateValueCb,
		  indicatorUpdateGraphicalInfoCb,
		  (XtPointer) pi);
		drawWhiteRectangle(pi->updateTask);
	    }
	}
  
      /* Update the limits to reflect current src's */
	updatePvLimits(&dlIndicator->limits);

      /* From the indicator structure, we've got Indicator's specifics */
	n = 0;
	XtSetArg(args[n],XtNx,(Position)dlIndicator->object.x); n++;
	XtSetArg(args[n],XtNy,(Position)dlIndicator->object.y); n++;
	XtSetArg(args[n],XtNwidth,(Dimension)dlIndicator->object.width); n++;
	XtSetArg(args[n],XtNheight,(Dimension)dlIndicator->object.height); n++;
	XtSetArg(args[n],XcNdataType,XcFval); n++;
      /* KE: Need to set these 3 values explicitly and not use the defaults
       *  because the widget is an XcLval by default and the default
       *  initializations are into XcVType.lval, possibly giving meaningless
       *  numbers in XcVType.fval, which is what will be used for our XcFval
       *  widget.  They still need to be set from the lval, however, because
       *  they are XtArgVal's, which Xt typedef's as long (exc. Cray?)
       *  See Intrinsic.h */
	XtSetArg(args[n],XcNincrement,longFval(0.)); n++;     /* Not used */
	XtSetArg(args[n],XcNlowerBound,longFval(dlIndicator->limits.lopr)); n++;
	XtSetArg(args[n],XcNupperBound,longFval(dlIndicator->limits.hopr)); n++;
	switch (dlIndicator->label) {
	case LABEL_NONE:
	case NO_DECORATIONS:
	    XtSetArg(args[n],XcNvalueVisible,FALSE); n++;
	    XtSetArg(args[n],XcNlabel," "); n++;
	    break;
	case OUTLINE:
	    XtSetArg(args[n],XcNvalueVisible,FALSE); n++;
	    XtSetArg(args[n],XcNlabel," "); n++;
	    break;
	case LIMITS:
	    XtSetArg(args[n],XcNvalueVisible,TRUE); n++;
	    XtSetArg(args[n],XcNlabel," "); n++;
	    break;
	case CHANNEL:
	    XtSetArg(args[n],XcNvalueVisible,TRUE); n++;
	    XtSetArg(args[n],XcNlabel,dlIndicator->monitor.rdbk); n++;
	    break;
	}

	switch (dlIndicator->direction) {
	  /*
	   * note that this is  "direction of increase"
	   */
	case UP:
	    XtSetArg(args[n],XcNscaleSegments,
	      (dlIndicator->object.width >INDICATOR_OKAY_SIZE ? 11 : 5)); n++;
	      XtSetArg(args[n],XcNorient,XcVert); n++;
	      if (dlIndicator->label == LABEL_NONE ||
		dlIndicator->label == NO_DECORATIONS) {
		  XtSetArg(args[n],XcNscaleSegments,0); n++;
	      }
	      break;
	case RIGHT:
	    XtSetArg(args[n],XcNscaleSegments,
	      (dlIndicator->object.height>INDICATOR_OKAY_SIZE ? 11 : 5)); n++;
	      XtSetArg(args[n],XcNorient,XcHoriz); n++;
	      if (dlIndicator->label == LABEL_NONE ||
		dlIndicator->label == NO_DECORATIONS) {
		  XtSetArg(args[n],XcNscaleSegments,0); n++;
	      }
	      break;
	}
	preferredHeight = dlIndicator->object.height/INDICATOR_FONT_DIVISOR;
	bestSize = dmGetBestFontWithInfo(fontTable,MAX_FONTS,NULL,
	  preferredHeight,0,&usedHeight,&usedCharWidth,FALSE);
	XtSetArg(args[n],XtNfont,fontTable[bestSize]); n++;

	XtSetArg(args[n],XcNindicatorForeground,(Pixel)
	  displayInfo->colormap[dlIndicator->monitor.clr]); n++;
	XtSetArg(args[n],XcNindicatorBackground,(Pixel)
	  displayInfo->colormap[dlIndicator->monitor.bclr]); n++;
	XtSetArg(args[n],XtNbackground,(Pixel)
	  displayInfo->colormap[dlIndicator->monitor.bclr]); n++;
	XtSetArg(args[n],XcNcontrolBackground,(Pixel)
	  displayInfo->colormap[dlIndicator->monitor.bclr]); n++;
      /*
       * add the pointer to the Channel structure as userData 
       *  to widget
       */
	XtSetArg(args[n],XcNuserData,(XtPointer)pi); n++;
	localWidget = XtCreateWidget("indicator", 
	  xcIndicatorWidgetClass, displayInfo->drawingArea, args, n);
	dlElement->widget = localWidget;
	
	if (displayInfo->traversalMode == DL_EDIT) {
	    addCommonHandlers(localWidget, displayInfo);
	    XtManageChild(localWidget);
	}
    } else {
	DlObject *po = &(dlElement->structure.indicator->object);
	XtVaSetValues(dlElement->widget,
	  XmNx, (Position)po->x,
	  XmNy, (Position)po->y,
	  XmNwidth, (Dimension)po->width,
	  XmNheight, (Dimension)po->height,
	  XcNlowerBound, longFval(dlIndicator->limits.lopr),
	  XcNupperBound, longFval(dlIndicator->limits.hopr),
	  XcNdecimals, (int)dlIndicator->limits.prec,
	  NULL);
    }
}

void hideDlIndicator(DisplayInfo *displayInfo, DlElement *dlElement)
{
  /* Use generic hide for an element with a widget */
    hideWidgetElement(displayInfo, dlElement);
}

static void indicatorDraw(XtPointer cd) {
    MedmIndicator *pi = (MedmIndicator *) cd;
    Record *pr = pi->record;
    Widget widget= pi->dlElement->widget;
    DlIndicator *dlIndicator = pi->dlElement->structure.indicator;
    XcVType val;

    if (pr->connected) {
	if (pr->readAccess) {
	    if (widget) {
		addCommonHandlers(widget, pi->updateTask->displayInfo);
		XtManageChild(widget);
	    } else {
		return;
	    }
	    val.fval = (float) pr->value;
	    XcIndUpdateValue(widget,&val);
	    switch (dlIndicator->clrmod) {
	    case STATIC :
	    case DISCRETE :
		break;
	    case ALARM :
		XcIndUpdateIndicatorForeground(widget,alarmColor(pr->severity));
		break;
	    }
	} else {
	    if (widget) XtUnmanageChild(widget);
	    draw3DPane(pi->updateTask,
	      pi->updateTask->displayInfo->colormap[dlIndicator->monitor.bclr]);
	    draw3DQuestionMark(pi->updateTask);
	}
    } else {
	if (widget) XtUnmanageChild(widget);
	drawWhiteRectangle(pi->updateTask);
    }
}

static void indicatorUpdateValueCb(XtPointer cd) {
    MedmIndicator *pi = (MedmIndicator *) ((Record *) cd)->clientData;
    updateTaskMarkUpdate(pi->updateTask);
}

static void indicatorUpdateGraphicalInfoCb(XtPointer cd) {
    Record *pr = (Record *) cd;
    MedmIndicator *pi = (MedmIndicator *) pr->clientData;
    DlIndicator *dlIndicator = pi->dlElement->structure.indicator;
    Widget widget = pi->dlElement->widget;
    Pixel pixel;
    XcVType hopr, lopr, val;
    short precision;

    switch (pr->dataType) {
    case DBF_STRING :
	medmPostMsg(1,"indicatorUpdateGraphicalInfoCb:\n"
	  "  Illegal channel type for %s\n"
	  "  Cannot attach Indicator\n",
	  dlIndicator->monitor.rdbk);
	return;
    case DBF_ENUM :
    case DBF_CHAR :
    case DBF_INT :
    case DBF_LONG :
    case DBF_FLOAT :
    case DBF_DOUBLE :
	hopr.fval = (float) pr->hopr;
	lopr.fval = (float) pr->lopr;
	val.fval = (float) pr->value;
	precision = pr->precision;
	break;
    default :
	medmPostMsg(1,"indicatorUpdateGraphicalInfoCb:\n"
	  "  Unknown channel type for %s\n"
	  "  Cannot attach Indicator\n",
	  dlIndicator->monitor.rdbk);
	break;
    }
    if ((hopr.fval == 0.0) && (lopr.fval == 0.0)) {
	hopr.fval += 1.0;
    }

    if (widget != NULL) {
      /* Set foreground pixel according to alarm */
	pixel = (dlIndicator->clrmod == ALARM) ?
	  alarmColor(pr->severity) :
	  pi->updateTask->displayInfo->colormap[dlIndicator->monitor.clr];
	XtVaSetValues(widget,
	  XcNindicatorForeground,pixel,
	  NULL);

      /* Set Channel and User limits (if apparently not set yet) */
	dlIndicator->limits.loprChannel = lopr.fval;
	if(dlIndicator->limits.loprSrc != PV_LIMITS_USER &&
	  dlIndicator->limits.loprUser == LOPR_DEFAULT) {
	    dlIndicator->limits.loprUser = lopr.fval;
	}
	dlIndicator->limits.hoprChannel = hopr.fval;
	if(dlIndicator->limits.hoprSrc != PV_LIMITS_USER &&
	  dlIndicator->limits.hoprUser == HOPR_DEFAULT) {
	    dlIndicator->limits.hoprUser = hopr.fval;
	}
	dlIndicator->limits.precChannel = precision;
	if(dlIndicator->limits.precSrc != PV_LIMITS_USER &&
	  dlIndicator->limits.precUser == PREC_DEFAULT) {
	    dlIndicator->limits.precUser = precision;
	}

      /* Set values in the widget if src is Channel */
	if(dlIndicator->limits.loprSrc == PV_LIMITS_CHANNEL) {
	    dlIndicator->limits.lopr = lopr.fval;
	    XtVaSetValues(widget, XcNlowerBound,lopr.lval, NULL);
	}
	if(dlIndicator->limits.hoprSrc == PV_LIMITS_CHANNEL) {
	    dlIndicator->limits.hopr = hopr.fval;
	    XtVaSetValues(widget, XcNupperBound,hopr.lval, NULL);
	}
	if(dlIndicator->limits.precSrc == PV_LIMITS_CHANNEL) {
	    dlIndicator->limits.prec = precision;
	    XtVaSetValues(widget,XcNdecimals, (int)precision, NULL);
	}
	XcIndUpdateValue(widget,&val);
    }
}

static void indicatorDestroyCb(XtPointer cd) {
    MedmIndicator *pi = (MedmIndicator *) cd;
    if (pi) {
	medmDestroyRecord(pi->record);
	free((char *)pi);
    }
    return;
}

static void indicatorGetRecord(XtPointer cd, Record **record, int *count) {
    MedmIndicator *pi = (MedmIndicator *) cd;
    *count = 1;
    record[0] = pi->record;
}

DlElement *createDlIndicator(DlElement *p) 
{
    DlIndicator *dlIndicator;
    DlElement *dlElement;

    dlIndicator = (DlIndicator *)malloc(sizeof(DlIndicator));
    if (!dlIndicator) return 0;
    if (p) {
	*dlIndicator = *p->structure.indicator;
    } else {
	objectAttributeInit(&(dlIndicator->object));
	monitorAttributeInit(&(dlIndicator->monitor));
	limitsAttributeInit(&(dlIndicator->limits));
	dlIndicator->label = LABEL_NONE;
	dlIndicator->clrmod = STATIC;
	dlIndicator->direction = RIGHT;
    }

    if (!(dlElement = createDlElement(DL_Indicator,
      (XtPointer)      dlIndicator,
      &indicatorDlDispatchTable))) {
	free(dlIndicator);
    }

    return(dlElement);
}

DlElement *parseIndicator(DisplayInfo *displayInfo)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlIndicator *dlIndicator;
    DlElement *dlElement = createDlIndicator(NULL);

    if (!dlElement) return 0;
    dlIndicator = dlElement->structure.indicator;

    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if (!strcmp(token,"object")) {
		parseObject(displayInfo,&(dlIndicator->object));
	    } else if (!strcmp(token,"monitor")) {
		parseMonitor(displayInfo,&(dlIndicator->monitor));
	    } else if (!strcmp(token,"label")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
		if (!strcmp(token,"none")) 
		  dlIndicator->label = LABEL_NONE;
		else if (!strcmp(token,"no decorations"))
		  dlIndicator->label = NO_DECORATIONS;
		else if (!strcmp(token,"outline"))
		  dlIndicator->label = OUTLINE;
		else if (!strcmp(token,"limits"))
		  dlIndicator->label = LIMITS;
		else if (!strcmp(token,"channel"))
		  dlIndicator->label = CHANNEL;
	    } else if (!strcmp(token,"clrmod")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
		if (!strcmp(token,"static")) 
		  dlIndicator->clrmod = STATIC;
		else if (!strcmp(token,"alarm"))
		  dlIndicator->clrmod = ALARM;
		else if (!strcmp(token,"discrete"))
		  dlIndicator->clrmod = DISCRETE;
	    } else if (!strcmp(token,"direction")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
		if (!strcmp(token,"up")) 
		  dlIndicator->direction = UP;
		else if (!strcmp(token,"right"))
		  dlIndicator->direction = RIGHT;
	      /* Backward compatibility */
		else if (!strcmp(token,"down"))
		  dlIndicator->direction = UP;
		else if (!strcmp(token,"left"))
		  dlIndicator->direction = RIGHT;
	    } else if (!strcmp(token,"limits")) {
	      parseLimits(displayInfo,&(dlIndicator->limits));
	    }
	    break;
	case T_EQUAL:
	    break;
	case T_LEFT_BRACE:
	    nestingLevel++; break;
	case T_RIGHT_BRACE:
	    nestingLevel--; break;
	}
    } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
      && (tokenType != T_EOF) );

    return dlElement;

}
void writeDlIndicator( FILE *stream, DlElement *dlElement, int level) {
/****************************************************************************
 * Write DL Indicator                                                       *
 ****************************************************************************/
    int i;
    char indent[16];
    DlIndicator *dlIndicator = dlElement->structure.indicator;

    for (i = 0;  i < level; i++) indent[i] = '\t';
    indent[i] = '\0';

    fprintf(stream,"\n%sindicator {",indent);
    writeDlObject(stream,&(dlIndicator->object),level+1);
    writeDlMonitor(stream,&(dlIndicator->monitor),level+1);
    if (dlIndicator->label != LABEL_NONE)
      fprintf(stream,"\n%s\tlabel=\"%s\"",indent,
        stringValueTable[dlIndicator->label]);
    if (dlIndicator->clrmod != STATIC)
      fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
        stringValueTable[dlIndicator->clrmod]);
#ifdef SUPPORT_0201XX_FILE_FORMAT
    if (dlIndicator->direction != RIGHT)
      fprintf(stream,"\n%s\tdirection=\"%s\"",indent,
        stringValueTable[dlIndicator->direction]);
#else
    if ((dlIndicator->direction != RIGHT) || (!MedmUseNewFileFormat))
      fprintf(stream,"\n%s\tdirection=\"%s\"",indent,
        stringValueTable[dlIndicator->direction]);
#endif
    writeDlLimits(stream,&(dlIndicator->limits),level+1);
    fprintf(stream,"\n%s}",indent);
}

static void indicatorInheritValues(ResourceBundle *pRCB, DlElement *p) {
    DlIndicator *dlIndicator = p->structure.indicator;
    medmGetValues(pRCB,
      RDBK_RC,       &(dlIndicator->monitor.rdbk),
      CLR_RC,        &(dlIndicator->monitor.clr),
      BCLR_RC,       &(dlIndicator->monitor.bclr),
      LABEL_RC,      &(dlIndicator->label),
      DIRECTION_RC,  &(dlIndicator->direction),
      CLRMOD_RC,     &(dlIndicator->clrmod),
      LIMITS_RC,     &(dlIndicator->limits),
      -1);
}

static void indicatorGetLimits(DlElement *pE, DlLimits **ppL, char **pN)
{
    DlIndicator *dlIndicator = pE->structure.indicator;

    *(ppL) = &(dlIndicator->limits);
    *(pN) = dlIndicator->monitor.rdbk;
}

static void indicatorGetValues(ResourceBundle *pRCB, DlElement *p) {
    DlIndicator *dlIndicator = p->structure.indicator;
    medmGetValues(pRCB,
      X_RC,          &(dlIndicator->object.x),
      Y_RC,          &(dlIndicator->object.y),
      WIDTH_RC,      &(dlIndicator->object.width),
      HEIGHT_RC,     &(dlIndicator->object.height),
      RDBK_RC,       &(dlIndicator->monitor.rdbk),
      CLR_RC,        &(dlIndicator->monitor.clr),
      BCLR_RC,       &(dlIndicator->monitor.bclr),
      LABEL_RC,      &(dlIndicator->label),
      DIRECTION_RC,  &(dlIndicator->direction),
      CLRMOD_RC,     &(dlIndicator->clrmod),
      LIMITS_RC,     &(dlIndicator->limits),
      -1);
}

static void indicatorSetBackgroundColor(ResourceBundle *pRCB, DlElement *p)
{
    DlIndicator *dlIndicator = p->structure.indicator;
    medmGetValues(pRCB,
      BCLR_RC,       &(dlIndicator->monitor.bclr),
      -1);
}

static void indicatorSetForegroundColor(ResourceBundle *pRCB, DlElement *p)
{
    DlIndicator *dlIndicator = p->structure.indicator;
    medmGetValues(pRCB,
      CLR_RC,        &(dlIndicator->monitor.clr),
      -1);
}
