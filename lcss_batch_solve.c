#include "lcss_batch_solve.h"

#define FALSE 0
#define TRUE 1

#define UP 2
#define DOWN 3
#define STOP 4

#define WRITE_LOG  

double epsilon = 0.0625;

char * aggregate_results_fname = "lcss_aggregate_results.csv";
char * optimal_value_log_fname = "optimal_value_log";
char * log_fname = "lcss_batch_solver_log";

/* old sweet spot decision */
/*
uint8_t decide(double norm_misses, double norm_queue_size){
    
    if(norm_misses > norm_queue_size){
        printf("normalized cache misses > normalized queue size, increase multiple\n");
        return UP;
    }
    if(norm_misses < norm_queue_size){
        printf("normalized cache misses < normalized queue size, decrease queue size\n");
        return DOWN;
    }
    if(norm_misses == norm_queue_size){
        return STOP;
    }
    printf("problem with decide function %f %f\n", norm_misses, norm_queue_size);
    exit(1);
}
*/


uint8_t decide(double norm_misses, double norm_queue_size){
    
    #ifdef WRITE_LOG
        FILE * fp;
    #endif
    
    if(norm_misses > 2.0){
        
        #ifdef WRITE_LOG
            fp = fopen(log_fname,"a");
            fprintf(fp,"normalized cache misses > 2.0, increase multiple\n");
            fclose(fp);
        #endif

        return UP;
    }
    if(norm_misses < 2.0){
        
        #ifdef WRITE_LOG
            fp = fopen(log_fname,"a");
            fprintf(fp,"normalized cache misses < 2.0, decrease queue size\n");
            fclose(fp);
        #endif
        
        return DOWN;
    }
    if(norm_misses == 2.0){
        return STOP;
    }
        
    #ifdef WRITE_LOG
        fp = fopen(log_fname,"a");
        fprintf(fp,"problem with decide function %f %f\n", norm_misses, norm_queue_size);
        fclose(fp);
    #endif
    
    exit(1);
}


void lcss_binary_search(int argc, char **argv){
    
    FILE * brfp, * fp;
    uint64_t n1, n2, n, MAX_TRIALS;
    double max_queue_size, min_queue_size;
    uint64_t table_size, queue_size, g, cache_misses, num_rebuilds;
    uint64_t * new_queue_size, cache_miss_threshold;
    int64_t length;
    uint8_t decision;
    double multiple, previous_multiple, difference;
    double probes_per_operation;
    double * norm_misses;
    double * norm_queue_size;
    double * goal_value;
    char * caching_strategy;
    char * instance_name;

    initialize_lcss(argc, argv);
    
    /* search interval for 51200 */
    min_queue_size = 80.0;
    max_queue_size = 135.0;
    
    MAX_TRIALS = 50;
    norm_misses = calloc(MAX_TRIALS, sizeof(double));
    norm_queue_size = calloc(MAX_TRIALS, sizeof(double));
        goal_value = calloc(MAX_TRIALS, sizeof(double));
    new_queue_size = calloc(MAX_TRIALS, sizeof(uint64_t));
    n1 = get_n1();
    n2 = get_n2();
    n = n1 < n2 ? n1 : n2;
    caching_strategy = get_caching_strategy();
    if(strcmp(caching_strategy, "lru") != 0){
        printf("lcss batch solver currently only works for lru\n");
        exit(1);
    }
    instance_name = get_instance_name();
    table_size = get_HASHTABLE_CAPACITY();
    queue_size = get_PRIORITY_QUEUE_CAPACITY();
    
    /* new sweet spot: what queue size needed for normalized misses = 2 */
    cache_miss_threshold = 2 * (n+1) * (n+1);
    
    #ifdef WRITE_LOG
        fp = fopen(log_fname,"a");
        fprintf(fp,"Setting cache miss threshold to %d\n", cache_miss_threshold);
        fclose(fp);
    #endif
    
    set_cache_miss_threshold( cache_miss_threshold );
    
    multiple = ( max_queue_size + min_queue_size ) / 2.0;
    previous_multiple = multiple + 1;
    
    for(g = 0; g < MAX_TRIALS; g++){
        
        new_queue_size[g] = (uint64_t)(multiple * (double)n);
        
        difference = multiple > previous_multiple ? multiple - previous_multiple : previous_multiple;
        
        if(difference <= epsilon){
            
            #ifdef WRITE_LOG
                fp = fopen(log_fname,"a");
                fprintf(fp,"Difference between multiple and previous multiple = %f. Stopping...\n", difference);
                fprintf(fp,"Midpoint is %f\n", (multiple+previous_multiple)/2.0 );
                fclose(fp);
            #endif

            break;
        }
        
/*
        if(g>0 && new_queue_size[g]==new_queue_size[g-1]){
            printf("No change in queue size! Stopping...\n");
            break;
        }
*/
        
        #ifdef WRITE_LOG
            fp = fopen(log_fname,"a");
            fprintf(fp,"Resetting queue size to %d\n", new_queue_size[g]);
            fclose(fp);
        #endif
        
        reset_lru_queue( new_queue_size[g] );
        reset_cache_misses();
        reset_subproblems();
        reset_num_lru_rebuilds();
        
        /* for old sweet spot */
/*
        cache_miss_threshold = multiple * (n+1) * (n+1);
*/
        
        set_threshold_reached(FALSE);
        
        #ifdef WRITE_LOG
            fp = fopen(log_fname,"a");
            fprintf(fp,"Conducting trial for multiple %f\n", multiple);
            fclose(fp);
        #endif

        /* Conduct trial */
        statistics.start_time = clock();
        length = lcss(n1-1, n2-1);
        statistics.elapsed_time = clock() - statistics.start_time;
        
        /* Find out what happened with this trial */
        cache_misses = get_cache_misses();
        norm_misses[g] = (double)cache_misses / (double)( (n1+1) * (n2+1) - 1 );

        queue_size = get_PRIORITY_QUEUE_CAPACITY();
        norm_queue_size[g] = (double)queue_size / (double)( n );

        goal_value[g] = norm_queue_size[g]-norm_misses[g];

        probes_per_operation = (double)get_probe_count() / (double)get_operation_count();
        num_rebuilds = get_num_rebuilds();
        
        /* Output information about this trial */
        
        #ifdef WRITE_LOG
            fp = fopen(log_fname,"a");
            fprintf(fp,"%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%d\n",
                    caching_strategy, instance_name, n1, n2, 
                    table_size, queue_size, cache_misses, length,
                    norm_misses[g], norm_queue_size[g], goal_value[g],
                    statistics.elapsed_time, probes_per_operation,
                    num_rebuilds);
            fclose(fp);
        #endif

        brfp = fopen(aggregate_results_fname,"a");
        fprintf(brfp, "%s,%s,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%d\n",
                caching_strategy, instance_name, n1, n2, table_size, 
                queue_size, cache_misses, length, norm_misses[g], norm_queue_size[g], 
                goal_value[g], statistics.elapsed_time,
                probes_per_operation, num_rebuilds);
        fclose(brfp);
        
        /* Decide whether to move up or down or stop */
        if(g>0 && goal_value[g]==goal_value[g-1]){
            
            #ifdef WRITE_LOG
                fp = fopen(log_fname,"a");
                fprintf(fp,"No change in goal value! Stopping...\n");
                fclose(fp);
            #endif

            break;
        }
        
        decision = decide(norm_misses[g], norm_queue_size[g]);
        if(decision == STOP){
            
            #ifdef WRITE_LOG
                fp = fopen(log_fname,"a");
                fprintf(fp,"optimal criteria was met! Stopping...\n");
                fclose(fp);
            #endif

            break;
        }
        if(decision == UP){
            min_queue_size = multiple;
            if(max_queue_size==multiple){
                
                #ifdef WRITE_LOG
                    fp = fopen(log_fname,"a");
                    printf("max equals multiple: stopping\n");
                    fclose(fp);
                #endif
                    
                break;
            }
            
            #ifdef WRITE_LOG
                fp = fopen(log_fname,"a");
                fprintf(fp,"Take midpoint of %f and %f\n", multiple, max_queue_size);
                fclose(fp);
            #endif
                
            previous_multiple = multiple;
            multiple = ( (double)max_queue_size + (double)multiple ) / 2.0;
        }
        if(decision == DOWN){
            max_queue_size = multiple;
            if(min_queue_size==multiple){
                
                #ifdef WRITE_LOG
                    fp = fopen(log_fname,"a");
                    fprintf(fp,"min equals multiple: stopping\n");
                    fclose(fp);
                #endif

                break;
            }
            
            #ifdef WRITE_LOG
                fp = fopen(log_fname,"a");
                fprintf(fp,"Take midpoint of %f and %f\n", min_queue_size, multiple);
                fclose(fp);
            #endif

            previous_multiple = multiple;
            multiple = ( (double)min_queue_size + (double)multiple ) / 2.0;
        }
        
        
        #ifdef WRITE_LOG
            fp = fopen(log_fname,"a");
            fprintf(fp,"===========================\n");
            fclose(fp);
        #endif

    }
    fp = fopen(optimal_value_log_fname, "a");
    fprintf(fp, "%s,%f\n", instance_name, (multiple+previous_multiple)/2.0 );
    fclose(fp);
}



void lcss_batch_solve(int argc, char **argv){

    lcss_binary_search(argc, argv);
}
