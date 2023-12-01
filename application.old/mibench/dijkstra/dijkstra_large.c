#include <system/service/logger.h>
#include "simple_alloc.h"
#include "test_data.h"

#define NUM_NODES 100
#define NONE 9999

struct _NODE {
    int iDist;
    int iPrev;
};
typedef struct _NODE NODE;

struct _QITEM {
    int iNode;
    int iDist;
    int iPrev;
    struct _QITEM *qNext;
};
typedef struct _QITEM QITEM;

QITEM *qHead = NULL;

int AdjMatrix[NUM_NODES][NUM_NODES];

int g_qCount = 0;
NODE rgnNodes[NUM_NODES];
int ch;
int iPrev, iNode;
int i, iCost, iDist;

void print_path(NODE *rgnNodes, int chNode) {
    if (rgnNodes[chNode].iPrev != NONE) {
        print_path(rgnNodes, rgnNodes[chNode].iPrev);
    }
    log_info(" " << dec << chNode);
}

void enqueue(int iNode, int iDist, int iPrev) {
    QITEM *qNew = (QITEM *)malloc(sizeof(QITEM));
    QITEM *qLast = qHead;

    if (!qNew) {
        log_error("Out of memory");
        while(1);
    }
    qNew->iNode = iNode;
    qNew->iDist = iDist;
    qNew->iPrev = iPrev;
    qNew->qNext = NULL;

    if (!qLast) {
        qHead = qNew;
    } else {
        while (qLast->qNext) qLast = qLast->qNext;
        qLast->qNext = qNew;
    }
    g_qCount++;
    //               ASSERT(g_qCount);
}

void dequeue(int *piNode, int *piDist, int *piPrev) {
    QITEM *qKill = qHead;

    if (qHead) {
        //                 ASSERT(g_qCount);
        *piNode = qHead->iNode;
        *piDist = qHead->iDist;
        *piPrev = qHead->iPrev;
        qHead = qHead->qNext;
        free(qKill);
        g_qCount--;
    }
}

int qcount(void) { return (g_qCount); }

void dijkstra(int chStart, int chEnd) {
    for (ch = 0; ch < NUM_NODES; ch++) {
        rgnNodes[ch].iDist = NONE;
        rgnNodes[ch].iPrev = NONE;
    }

    if (chStart == chEnd) {
        log_info("Shortest path is 0 in cost. Just stay where you are.");
    } else {
        rgnNodes[chStart].iDist = 0;
        rgnNodes[chStart].iPrev = NONE;

        enqueue(chStart, 0, NONE);

        while (qcount() > 0) {
            dequeue(&iNode, &iDist, &iPrev);
            for (i = 0; i < NUM_NODES; i++) {
                if ((iCost = AdjMatrix[iNode][i]) != NONE) {
                    if ((NONE == rgnNodes[i].iDist) ||
                        (rgnNodes[i].iDist > (iCost + iDist))) {
                        rgnNodes[i].iDist = iDist + iCost;
                        rgnNodes[i].iPrev = iNode;
                        enqueue(i, iDist + iCost, iNode);
                    }
                }
            }
        }

        log_info("Shortest path is "<< dec << rgnNodes[chEnd].iDist << " in cost. ");
        log_info("Path is: ");
        print_path(rgnNodes, chEnd);
        log_info("\n");
    }
}

void run_disjkstra() {
    int i, j;

    /* make a fully connected matrix */
    for (i = 0; i < NUM_NODES; i++) {
        for (j = 0; j < NUM_NODES; j++) {
            /* make it more sparce */
            AdjMatrix[i][j] = dijkstra_input_data[i][j];
        }
    }

    /* finds 10 shortest paths between nodes */
    for (i = 0, j = NUM_NODES / 2; i < 100; i++, j++) {
        j = j % NUM_NODES;
        dijkstra(i, j);
    }
}
