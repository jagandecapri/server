/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: expandtab:ts=8:sw=4:softtabstop=4:
#ifndef FT_CACHETABLE_WRAPPERS_H
#define FT_CACHETABLE_WRAPPERS_H

#ident "$Id$"
#ident "Copyright (c) 2007-2011 Tokutek Inc.  All rights reserved."
#ident "The technology is licensed by the Massachusetts Institute of Technology, Rutgers State University of New Jersey, and the Research Foundation of State University of New York at Stony Brook under United States of America Serial No. 11/760379 and to the patents and/or patent applications resulting from it."

#include <fttypes.h>
#include "cachetable.h"

/**
 * Put an empty node (that is, no fields filled) into the cachetable. 
 * In the process, write dependent nodes out for checkpoint if 
 * necessary.
 */
void
cachetable_put_empty_node_with_dep_nodes(
    FT h,
    u_int32_t num_dependent_nodes,
    FTNODE* dependent_nodes,
    BLOCKNUM* name, //output
    u_int32_t* fullhash, //output
    FTNODE* result
    );

/**
 * Create a new ftnode with specified height and number of children.
 * In the process, write dependent nodes out for checkpoint if 
 * necessary.
 */
void
create_new_ftnode_with_dep_nodes(
    FT h,
    FTNODE *result,
    int height,
    int n_children,
    u_int32_t num_dependent_nodes,
    FTNODE* dependent_nodes
    );

/**
 * Create a new ftnode with specified height
 * and children. 
 * Used for test functions only.
 */
void
toku_create_new_ftnode (
    FT_HANDLE t,
    FTNODE *result,
    int height,
    int n_children
    );

/**
 * toku_pin_ftnode either pins a ftnode, if the operation is fast (because
 * a partial fetch is not required and there is no contention for the node)
 * or it returns TOKUDB_TRY_AGAIN after unlocking its ancestors (using 
 * unlockers and ancestors) and bringing the necessary pieces of the node
 * into memory. 
 */
int
toku_pin_ftnode(
    FT_HANDLE brt,
    BLOCKNUM blocknum,
    u_int32_t fullhash,
    UNLOCKERS unlockers,
    ANCESTORS ancestors,
    const PIVOT_BOUNDS pbounds,
    FTNODE_FETCH_EXTRA bfe,
    BOOL may_modify_node,
    BOOL apply_ancestor_messages, // this BOOL is probably temporary, for #3972, once we know how range query estimates work, will revisit this
    FTNODE *node_p,
    BOOL* msgs_applied
    );

/**
 * Unfortunately, this function is poorly named
 * as over time, client threads have also started
 * calling this function.
 * This function returns a pinned ftnode to the caller.
 * Unlike toku_pin_ftnode, this function blocks until the node is pinned.
 */
void
toku_pin_ftnode_off_client_thread(
    FT h,
    BLOCKNUM blocknum,
    u_int32_t fullhash,
    FTNODE_FETCH_EXTRA bfe,
    BOOL may_modify_node,
    u_int32_t num_dependent_nodes,
    FTNODE* dependent_nodes,
    FTNODE *node_p
    );

/**
 * Effect: Unpin a brt node. Used for
 * nodes that were pinned off client thread.
 */
void
toku_unpin_ftnode_off_client_thread(FT h, FTNODE node);

/**
 * Effect: Unpin a brt node.
 * Used for nodes pinned on a client thread
 */
void
toku_unpin_ftnode(FT h, FTNODE node);

void
toku_unpin_ftnode_read_only(FT_HANDLE brt, FTNODE node);

#endif