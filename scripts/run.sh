# Usage: 

# lcss100 
#     -h <hashing-algorithm>         (default: MurmurHash32)
#     -t <table-size-exponent>       (default: 8)
#     -c <collision-resolution-type> (default: linear_probe)
#     -s <caching-strategy>          (default: hashing)
#     -i <instance-name>             (default: {(0,0,0), (1,1,1)})


# Hashing algorithm: {MurmurHash32, MurmurHash64A}
# Collision resolution type: {linear_probe, cuckoo}
# Caching strategy: {non_memo, hashing, LRU, NRU}

./lcss_batch_solver -s lru -i ../lcss_instances/r/200/random-200-10-0
