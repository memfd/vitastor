// Copyright (c) Vitaliy Filippov, 2019+
// License: VNPL-1.1 (see README.md for details)

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "osd_primary_chain_check.h"

struct test_inode_config_t
{
    inode_t num = 0;
    inode_t parent_id = 0;
    uint64_t mod_revision = 0;
};

static test_inode_config_t make_inode(inode_t num, inode_t parent_id, uint64_t mod_revision)
{
    test_inode_config_t cfg;
    cfg.num = num;
    cfg.parent_id = parent_id;
    cfg.mod_revision = mod_revision;
    return cfg;
}

int main(int argc, char *argv[])
{
    std::map<inode_t, test_inode_config_t> inode_config;
    std::vector<inode_t> read_chain;
    inode_t inode = INODE_WITH_POOL(1, 1);
    inode_t parent = INODE_WITH_POOL(1, 2);
    inode_t foreign_parent = INODE_WITH_POOL(2, 3);

    int r = collect_chained_read_inodes(inode_config, inode, 1, 10, read_chain);
    assert(r == -EPIPE);
    assert(read_chain.empty());

    inode_config[inode] = make_inode(inode, parent, 10);
    r = collect_chained_read_inodes(inode_config, inode, 1, 9, read_chain);
    assert(r == -EPIPE);
    assert(read_chain.empty());

    r = collect_chained_read_inodes(inode_config, inode, 1, 10, read_chain);
    assert(r == -EPIPE);
    assert(read_chain.empty());

    inode_config[parent] = make_inode(parent, 0, 11);
    r = collect_chained_read_inodes(inode_config, inode, 1, 10, read_chain);
    assert(r == 0);
    assert(read_chain.size() == 2);
    assert(read_chain[0] == inode);
    assert(read_chain[1] == parent);

    inode_config[parent] = make_inode(parent, inode, 11);
    r = collect_chained_read_inodes(inode_config, inode, 1, 10, read_chain);
    assert(r == -EINVAL);
    assert(read_chain.empty());

    inode_config[inode] = make_inode(inode, foreign_parent, 10);
    inode_config[parent] = make_inode(parent, 0, 11);
    r = collect_chained_read_inodes(inode_config, inode, 1, 10, read_chain);
    assert(r == 0);
    assert(read_chain.size() == 1);
    assert(read_chain[0] == inode);

    printf("ok\n");
    return 0;
}
