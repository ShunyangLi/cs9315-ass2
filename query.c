// query.c ... query scan functions
// part of Multi-attribute Linear-hashed Files
// Manage creating and using Query objects
// Last modified by John Shepherd, July 2019

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"

#include "bits.h"
#include "hash.h"

// A suggestion ... you can change however you like

struct QueryRep {
	Reln    rel;       // need to remember Relation info
	Bits    known;     // the hash value from MAH
	Bits    unknown;   // the unknown bits from MAH
	PageID  curpage;   // current page in scan
	int     is_ovflow; // are we in the overflow pages?
	Offset  curtup;    // offset of current tuple within page
	//TODO:
	Tuple   queryString;
	//Count   tupleNum;
	PageID  overflow;
};

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan

Query startQuery(Reln r, char *q)
{
    // TODO:
    // Partial algorithm:
    // form known bits from known attributes
    // form unknown bits from '?' attributes
    // compute PageID of first page
    //   using known bits and first "unknown" value
    // set all values in QueryRep object

	Query query = malloc(sizeof(struct QueryRep));
	assert(query != NULL);
    query->rel = r;
    query->is_ovflow = FALSE;
    query->curtup = 0;
    query->queryString = q;

	// init the attr
	Bits pageId;
	int DataDepth = depth(r);
	ChVecItem *cv = chvec(r);
	Count nvals = nattrs(r);
	char **vals = malloc(nvals*sizeof(char *));
	assert(vals != NULL);
	tupleVals(q, vals);

	Bits hash[nvals + 1];
	Bits knownValue = 0;
	Bits unknownValue = 0;
	int i = 0;

	// hash all the value, if 0, then was unknown, otherwise is known
	for (i = 0; i < nvals; i ++) {
		if (!strcmp(vals[i], "?")) hash[i] = 0;
		else hash[i] = hash_any((unsigned char *)vals[i],strlen(vals[i]));
	}

	for (i = 0; i < MAXBITS; i ++) {
		Bits att = cv[i].att;
		Bits bit = cv[i].bit;
		// form the known value
		if (bitIsSet(hash[att],bit)) knownValue = setBit(knownValue,i);
		// form the unknown value
		if (!strcmp(vals[att], "?")) unknownValue = setBit(unknownValue,i);
	}

	// setup the unknown and knwon values
    query->known = knownValue;
    query->unknown = unknownValue;


    Bits lowerValue = getLower(knownValue, DataDepth);
    Bits lowerNext = getLower(knownValue, DataDepth + 1);

    // printf("here: %u, %u\n",lowerValue,lowerNext);
    // get the page values
    pageId = (lowerValue < splitp(r))?lowerNext:lowerValue;

	query->curpage = pageId;
	query->overflow = 0;

	// query->tupleNum = ZERO;
	free(vals);
	return query;
}

// get next tuple during a scan

Tuple getNextTuple(Query q)
{
	// TODO: 
	// Partial algorithm:
	// if (more tuples in current page)
	//    get next matching tuple from current page
	// else if (current page has overflow)
	//    move to overflow page
	//    grab first matching tuple from page
	// else
	//    move to "next" bucket
	//    grab first matching tuple from data page
	// endif
	// if (current page has no matching tuples)
	//    go to next page (try again)
	// endif
	// Page page = getPage(dataFile(q->rel),q->curpage);

	// set the page according to whether overflow
	// if (q->is_ovflow) page = getPage(ovflowFile(q->rel),q->curpage);

	// get the page according to the is overflow
    Page page = getPage(dataFile(q->rel),q->curpage);
    if (q->is_ovflow) page = getPage(ovflowFile(q->rel), q->overflow);

    Count offset = pageOffset(page);
    // check whether in current page
	for (;q->curtup < offset;) {
		Tuple tuple = pageData(page) + q->curtup;
		Count tuplelength = tupLength(tuple);
		q->curtup += tuplelength + 1;

		// if we can find the match, then we got the result
		if (tupleMatch(q->rel, tuple,q->queryString)) {
			return copyString(tuple);
		}
	}

	if (pageOvflow(page) != NO_PAGE) {
		// reset the query
        q->is_ovflow = TRUE;
		// q->curpage = pageOvflow(page);
		q->overflow = pageOvflow(page);
		q->curtup = 0;
		// q->tupleNum = 0;
		// printf("OverFlow\n\n");
		// iterator again
        return getNextTuple(q);
	} else {

	    int index  = 0;
	    Bits nextPage = q->curpage;
	    Bits unknownValue = q->unknown;
	    // Bits known = q->known;
	    Reln r = q->rel;

	    while (index < MAXBITS) {
	        if (bitIsSet(unknownValue, index)) {
	            if (bitIsSet(nextPage, index)) {
	                nextPage = unsetBit(nextPage, index);
	            } else {
	                nextPage = setBit(nextPage,index);
                    break;
	            }
	        }
            index++;
	    }

	    // check the next page range, if greater than current, then replace
        if (nextPage > q->curpage && nextPage < npages(r)) {
	        q->curpage = nextPage;
	        q->is_ovflow = FALSE;
	        q->overflow = 0;
	        q->curtup = 0;
	        free(page);
            return getNextTuple(q);
	    }

    }

	if (page != NULL) free(page);
	return NULL;
}

// clean up a QueryRep object and associated data
void closeQuery(Query q)
{
	// TODO: 
	free(q);
}
