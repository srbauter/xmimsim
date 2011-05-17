/*
Copyright (C) 2010-2011 Tom Schoonjans and Laszlo Vincze

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "xmi_xslt.h"
#include "xmi_xml.h"
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/catalog.h>
#include <libxml/xpathInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>


extern int xmlLoadExtDtdDefaultValue;

static void update_xpath_nodes(xmlNodeSetPtr nodes, const xmlChar* value) {
	int size;
	int i;
	   
	size = (nodes) ? nodes->nodeNr : 0;
	for(i = size - 1; i >= 0; i--) {
		xmlNodeSetContent(nodes->nodeTab[i], value);

		if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL)
	    		nodes->nodeTab[i] = NULL;
	}
}		


int xmi_xmso_to_xmsi_xslt(char *xmsofile, char *xmsifile , char *outputfile  ) {

	xsltStylesheetPtr cur = NULL;
	xmlDocPtr doc, res;
	const char *params[1] = {NULL};
	const xmlChar xsltfile[] = XMI_XMSO2XMSI_XSLT;
	xmlXPathContextPtr xpathCtx; 
	xmlXPathObjectPtr xpathObj;


	xsltInit();

	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;

	cur = xsltParseStylesheetFile(xsltfile);
	if (cur == NULL)
		return 0;
	doc = xmlParseFile(xmsofile);
	if (doc == NULL)
		return 0;


	res = xsltApplyStylesheet(cur, doc, params);
	if (res == NULL)
		return 0;

	if (outputfile != NULL) {
    		xpathCtx = xmlXPathNewContext(res);
        	if(xpathCtx == NULL) {
	        	fprintf(stderr,"Error: unable to create new XPath context\n");
		        return 0;
		}
		xpathObj = xmlXPathEvalExpression((const xmlChar *) "/xmimsim/general/outputfile", xpathCtx);
		    if(xpathObj == NULL) {
		        fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n","xmimsim/general/outputfile" );
			xmlXPathFreeContext(xpathCtx); 
			return 0;
		}
		update_xpath_nodes(xpathObj->nodesetval, (const xmlChar *) outputfile);
		xmlXPathFreeObject(xpathObj);
	        xmlXPathFreeContext(xpathCtx);
	}


	xsltSaveResultToFilename(xmsifile, res, cur, 0);

	xsltFreeStylesheet(cur);
	xmlFreeDoc(res);
	xmlFreeDoc(doc);

        xsltCleanupGlobals();
        xmlCleanupParser();

	return 1;

}
