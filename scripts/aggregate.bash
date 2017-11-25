num_instances=5
min=5
for n in 51200
do

  # for misses <= 2
  threshold=$(((($n+1)*($n+1)-1)*2))
  echo $threshold

  #./lcss100 --caching_strategy linear_probe_hashing -i ../lcss_instances/t/test$n-000-111 --lru_queue_size $n
  for (( j=10; j<=10; j++ ))
  do
    for (( k=0; k<$num_instances; k++ ))
    do
      for (( g=86; g<=86; g++ ))
      do
        queue_size=$(($g * $n))
        instance=random-$n-$j-$k
        #instance=permutation$n-$j-$k
        #echo $instance $queue_size
        #echo ./lcss100 --caching_strategy linear_probe_hashing -i ../lcss_instances/r/$n/$instance
        #./lcss100 -s hashing -i ../lcss_instances/r/$n/$instance
        # used to be -t 25 meaning 2^25 for the cache size now we use --cache_size 33554432
        echo ./lcss_batch_solver --caching_strategy lru -i ../lcss_instances/r/$n/$instance --lru_queue_size $queue_size --cache_size 33554432 -threshold $threshold
        ./lcss_batch_solver --caching_strategy lru -i ../lcss_instances/r/$n/$instance --lru_queue_size $queue_size --cache_size 33554432 -threshold $threshold
      done
    done
  done

  num_instances=$(( $num_instances / 2 ))
  if [ $num_instances -lt $min ] ;
  then
    num_instances=5;
  fi
  echo $num_instances

done

# solving n=12800 five times took time: (real) 429m36.751s, (user) 428m43.600s, (sys) 0m13.360s
