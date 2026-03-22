// Copyright (c) Vitaliy Filippov, 2019+
// License: VNPL-1.1 (see README.md for details)

#pragma once

#include <algorithm>
#include <errno.h>
#include <map>
#include <vector>

#include "object_id.h"

// Returns 0 and fills <read_chain> with the current inode followed by same-pool parents.
// Returns -EPIPE when the OSD metadata view is stale or incomplete and -EINVAL on parent loops.
//
// InodeCfg is expected to expose:
// - num
// - parent_id
// - mod_revision
template<typename InodeMap>
int collect_chained_read_inodes(
    const InodeMap &inode_config,
    inode_t inode,
    pool_id_t pool_id,
    uint64_t meta_revision,
    std::vector<inode_t> &read_chain
)
{
    read_chain.clear();
    auto inode_it = inode_config.find(inode);
    if (inode_it == inode_config.end() || inode_it->second.mod_revision != meta_revision)
    {
        return -EPIPE;
    }
    read_chain.push_back(inode);
    while (inode_it->second.parent_id && INODE_POOL(inode_it->second.parent_id) == pool_id)
    {
        inode_t parent_id = inode_it->second.parent_id;
        if (std::find(read_chain.begin(), read_chain.end(), parent_id) != read_chain.end())
        {
            read_chain.clear();
            return -EINVAL;
        }
        auto parent_it = inode_config.find(parent_id);
        if (parent_it == inode_config.end())
        {
            read_chain.clear();
            return -EPIPE;
        }
        read_chain.push_back(parent_id);
        inode_it = parent_it;
    }
    return 0;
}
