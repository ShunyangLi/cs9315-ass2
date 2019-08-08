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
    Bits 	hashVal;
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
	Bits knownValue = ZERO;
	Bits unknownValue = ZERO;
	int i = 0;

	// hash all the value, if 0, then was unknown, otherwise is known
	for (i = 0; i < nvals; i ++) {
		if (!strcmp(vals[i], "?")) hash[i] = ZERO;
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
    query->hashVal = knownValue;


    Bits lowerValue = getLower(knownValue, DataDepth);
    Bits lowerNext = getLower(knownValue, DataDepth + ONE);

    // printf("here: %u, %u\n",lowerValue,lowerNext);
    // get the page values
    pageId = (lowerValue < splitp(r))?lowerNext:lowerValue;

	query->curpage = pageId;
	query->overflow = ZERO;

	// query->tupleNum = ZERO;
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

	//  printf("overflow: %u, %u\n", q->overflow, q->curpage);

    Page page = getPage(dataFile(q->rel),q->curpage);
    if (q->is_ovflow) page = getPage(ovflowFile(q->rel), q->overflow);

    Count offset = pageOffset(page);
    // check whether in current page
	for (;q->curtup < offset;) {
		Tuple tuple = pageData(page) + q->curtup;
		Count tuplelength = tupLength(tuple);
		q->curtup += tuplelength + ONE;
        // printf("page: %u\n", q->curpage);
		// if we can find the match, then we got the result
		if (tupleMatch(q->rel, tuple,q->queryString)) {
			return tuple;
		}
	}

	if (pageOvflow(page) != NO_PAGE) {
		// reset the query
        q->is_ovflow = TRUE;
		// q->curpage = pageOvflow(page);
		q->overflow = pageOvflow(page);
		q->curtup = ZERO;
		// q->tupleNum = 0;
		// printf("OverFlow\n\n");
		// iterator again
        return getNextTuple(q);
	} else {

	    int index  = 0;
	    Bits hash = q->curpage;
	    Bits unknwon = q->unknown;
	    // Bits known = q->known;
	    Reln r = q->rel;
	    Count sd = (1<<depth(r)) + splitp(r);

	    while (index < sd) {
	        if (bitIsSet(unknwon, index)) {
	            if (bitIsSet(hash, index)) {
	                hash = unsetBit(hash, index);
	            } else {
	                hash = setBit(hash,index);
                    break;
	            }
	        }
            index++;
	    }

//	    hash = getLower(hash,depth(r));
//	    if (hash < splitp(r)) hash = getLower(hash,depth(r)+1);

	    // printf("page: %u,%u\n",hash, q->curpage);
        if (hash > q->curpage && hash < npages(r)) {
	        q->curpage = hash;
	        q->is_ovflow = FALSE;
	        q->overflow = ZERO;
	        q->curtup = ZERO;
            return getNextTuple(q);
	    }

    }

	return NULL;
}

// clean up a QueryRep object and associated data
void closeQuery(Query q)
{
	// TODO: 
	free(q);
}
