/* applications/stress-ng/stress_stored_job.c */

/*
 * ===========================================================================
 * Stress-NG Built-in Job Definitions
 * ===========================================================================
 * Mode: Production (Target execution time based on --ops)
 *
 * Strategy:
 * - Ops calculated as: (Measured Ops in 10m run * 0.8) / Concurrency.
 * - Rounded to nearest convenient number.
 * - Dentry: Reduced concurrency (-c 2) for high stages to prevent timeout.
 * - Debug Mode: 1/10th of Production Ops.
 */

#if 1  /* <--- Set to 0 to enable Fast Debug mode (1/10th ops) below */

/*
 * ===========================================================================
 * [ACTIVE] Production Configuration (Target ~10 Minutes)
 * ===========================================================================
 */

const char JOB_DATA_CPU[] =
    /* --- Stage 1: 20% --- */
    "cpu       --ops 2800    -c 1 --cpu-load 20\n"
    "matrix    --ops 6400    -c 1 --matrix-size 32\n"
    "qsort     --ops 2500    -c 1 --qsort-size 1024\n"
    /* --- Rest --- */
    "atomic    --ops 3000000 -c 1 --atomic-thread 1\n"
    "bitops    --ops 24000   -c 1\n"
    "bsearch   --ops 3100    -c 1 --bsearch-size 1024\n"
    "context   --ops 690000  -c 1 --context-threads 1\n"
    "fp        --ops 1050    -c 1\n"
    "prime     --ops 2700000 -c 1\n"
    "stack     --ops 6900    -c 1 --stack-size 4096\n"
    "str       --ops 1300000 -c 1 --str-size 1024\n"
    "trig      --ops 210     -c 1\n"
    "vecmath   --ops 2700    -c 1\n"

    /* --- Stage 2: 40% --- */
    "cpu       --ops 4900    -c 2 --cpu-load 40\n"
    "matrix    --ops 2800    -c 2 --matrix-size 64\n"
    "qsort     --ops 690     -c 2 --qsort-size 4096\n"
    /* --- Rest --- */
    "atomic    --ops 3300000 -c 2 --atomic-thread 2\n"
    "bitops    --ops 25000   -c 2\n"
    "bsearch   --ops 1200    -c 2 --bsearch-size 4096\n"
    "context   --ops 440000  -c 2 --context-threads 2\n"
    "fp        --ops 730     -c 2\n"
    "prime     --ops 2100000 -c 2\n"
    "stack     --ops 6700    -c 2 --stack-size 8192\n"
    "str       --ops 375000  -c 2 --str-size 4096\n"
    "trig      --ops 160     -c 2\n"
    "vecmath   --ops 1900    -c 2\n"

    /* --- Stage 3: 60% --- */
    "cpu       --ops 4500    -c 3 --cpu-load 60\n"
    "matrix    --ops 1600    -c 3 --matrix-size 64\n"
    "qsort     --ops 100     -c 3 --qsort-size 16384\n"
    /* --- Rest --- */
    "atomic    --ops 1800000 -c 3 --atomic-thread 2\n"
    "bitops    --ops 14000   -c 3\n"
    "bsearch   --ops 140     -c 3 --bsearch-size 16384\n"
    "context   --ops 210000  -c 3 --context-threads 2\n"
    "fp        --ops 400     -c 3\n"
    "prime     --ops 1300000 -c 3\n"
    "stack     --ops 3700    -c 3 --stack-size 16384\n"
    "str       --ops 155000  -c 3 --str-size 8192\n"
    "trig      --ops 120     -c 3\n"
    "vecmath   --ops 1300    -c 3\n"

    /* --- Stage 4: 80% --- */
    "cpu       --ops 4400    -c 4 --cpu-load 80\n"
    "matrix    --ops 280     -c 4 --matrix-size 128\n"
    "qsort     --ops 45      -c 4 --qsort-size 32768\n"
    /* --- Rest --- */
    "atomic    --ops 1400000 -c 4 --atomic-thread 4\n"
    "bitops    --ops 10500   -c 4\n"
    "bsearch   --ops 55      -c 4 --bsearch-size 32768\n"
    "context   --ops 150000  -c 4 --context-threads 3\n"
    "fp        --ops 320     -c 4\n"
    "prime     --ops 1100000 -c 4\n"
    "stack     --ops 2700    -c 4 --stack-size 32768\n"
    "str       --ops 165000  -c 4 --str-size 8192\n"
    "trig      --ops 125     -c 4\n"
    "vecmath   --ops 1350    -c 4\n"

    /* --- Stage 5: 100% --- */
    "cpu       --ops 4100    -c 5 --cpu-load 100\n"
    "matrix    --ops 190     -c 5 --matrix-size 128\n"
    "qsort     --ops 15      -c 5 --qsort-size 65536\n"
    /* --- Rest --- */
    "atomic    --ops 1200000 -c 5 --atomic-thread 4\n"
    "bitops    --ops 8800    -c 5\n"
    "bsearch   --ops 20      -c 5 --bsearch-size 65536\n"
    "context   --ops 130000  -c 5 --context-threads 3\n"
    "fp        --ops 260     -c 5\n"
    "prime     --ops 800000  -c 5\n"
    "stack     --ops 1350    -c 5 --stack-size 65536\n"
    "str       --ops 25000   -c 5 --str-size 16384\n"
    "trig      --ops 105     -c 5\n"
    "vecmath   --ops 1100    -c 5\n";

const char JOB_DATA_MEMORY[] =
    /* --- Stage 1: 20% --- */
    "memcpy    --ops 15000    -c 1 --memcpy-size 32768\n"
    "stream    --ops 52000    -c 1 --stream-elem 2048\n"
    "vm        --ops 42000000 -c 1 --vm-bytes 1048576\n"
    "malloc    --ops 1700000  -c 1 --malloc-bytes 256\n"
    "memthrash --ops 2300     -c 1 --mem-size 65536\n"
    "ptr-chase --ops 86000000 -c 1 --ptr-chase-pages 128\n"

    /* --- Stage 2: 40% --- */
    "memcpy    --ops 5300     -c 2 --memcpy-size 65536\n"
    "stream    --ops 20500    -c 2 --stream-elem 4096\n"
    "vm        --ops 37500000 -c 2 --vm-bytes 4194304\n"
    "malloc    --ops 1400000  -c 2 --malloc-bytes 1024\n"
    "memthrash --ops 2200     -c 2 --mem-size 131072\n"
    "ptr-chase --ops 62000000 -c 2 --ptr-chase-pages 256\n"

    /* --- Stage 3: 60% --- */
    "memcpy    --ops 2300     -c 3 --memcpy-size 131072\n"
    "stream    --ops 9000     -c 3 --stream-elem 8192\n"
    "vm        --ops 39000000 -c 2 --vm-bytes 2097152\n"
    "malloc    --ops 920000   -c 3 --malloc-bytes 4096\n"
    "memthrash --ops 1800     -c 3 --mem-size 131072\n"
    "ptr-chase --ops 27500000 -c 3 --ptr-chase-pages 512\n"

    /* --- Stage 4: 80% --- */
    "memcpy    --ops 700      -c 4 --memcpy-size 262144\n"
    "stream    --ops 2700     -c 4 --stream-elem 16384\n"
    "vm        --ops 37500000 -c 2 --vm-bytes 4194304\n"
    "malloc    --ops 660000   -c 4 --malloc-bytes 8192\n"
    "memthrash --ops 1100     -c 4 --mem-size 262144\n"
    "ptr-chase --ops 20000000 -c 4 --ptr-chase-pages 256\n"

    /* --- Stage 5: 100% --- */
    "memcpy    --ops 280      -c 5 --memcpy-size 524288\n"
    "stream    --ops 1100     -c 5 --stream-elem 32768\n"
    "vm        --ops 29000000 -c 3 --vm-bytes 8388608\n"
    "malloc    --ops 400000   -c 5 --malloc-bytes 16384\n"
    "memthrash --ops 900      -c 5 --mem-size 524288\n"
    "ptr-chase --ops 26000000 -c 5 --ptr-chase-pages 128\n";

const char JOB_DATA_FILE[] =
    /* --- Stage 1: 20% --- */
    "hdd       --ops 420  -c 1 --hdd-bytes 131072\n"
    "open      --ops 2500 -c 1 --open-max 32\n"
    "copy-file --ops 2200 -c 1 --copy-file-bytes 131072\n"
    "unlink    --ops 2500 -c 1\n"
    "fstat     --ops 6000 -c 1 --fstat-files 10\n"
    "dentry    --ops 3000 -c 1 --dentries 32\n"
    "rename    --ops 2000 -c 1\n"
    "pipe      --ops 1000 -c 1 --pipe-data-size 512\n"

    /* --- Stage 2: 40% --- */
    "hdd       --ops 210  -c 2 --hdd-bytes 524288\n"
    "open      --ops 1200 -c 2 --open-max 64\n"
    "copy-file --ops 820  -c 2 --copy-file-bytes 524288\n"
    "unlink    --ops 1100 -c 2\n"
    "fstat     --ops 2400 -c 2 --fstat-files 10\n"
    "dentry    --ops 1300 -c 2 --dentries 64\n"
    "rename    --ops 1000 -c 2\n"
    "pipe      --ops 200  -c 2 --pipe-data-size 4096\n"

    /* --- Stage 3: 60% --- */
    "hdd       --ops 150  -c 3 --hdd-bytes 1048576\n"
    "open      --ops 800  -c 3 --open-max 50\n"
    "copy-file --ops 450  -c 3 --copy-file-bytes 1048576\n"
    "unlink    --ops 700  -c 3\n"
    "fstat     --ops 1500 -c 3 --fstat-files 8\n"
    "dentry    --ops 800  -c 3 --dentries 128\n"
    "rename    --ops 700  -c 3\n"
    "pipe      --ops 300  -c 3 --pipe-data-size 32768\n"

    /* --- Stage 4: 80% --- */
    "hdd       --ops 120  -c 4 --hdd-bytes 2097152\n"
    "open      --ops 800  -c 4 --open-max 40\n"
    "copy-file --ops 190  -c 4 --copy-file-bytes 2097152\n"
    "unlink    --ops 500  -c 4\n"
    "fstat     --ops 1200 -c 4 --fstat-files 6\n"
    "dentry    --ops 600  -c 2 --dentries 128\n"
    "rename    --ops 350  -c 4\n"
    "pipe      --ops 300  -c 4 --pipe-data-size 32768\n"

    /* --- Stage 5: 100% --- */
    "hdd       --ops 100  -c 5 --hdd-bytes 4194304\n"
    "open      --ops 700  -c 5 --open-max 40\n"
    "copy-file --ops 5    -c 5 --copy-file-bytes 4194304\n"
    "unlink    --ops 400  -c 5\n"
    "fstat     --ops 1100 -c 5 --fstat-files 5\n"
    "dentry    --ops 600  -c 2 --dentries 128\n"
    "rename    --ops 300  -c 5\n"
    "pipe      --ops 300  -c 5 --pipe-data-size 32768\n";

#else

/*
 * ===========================================================================
 * [DISABLED] Fast Debug Configuration (Target ~1 Minute)
 * ===========================================================================
 * Scaled to 1/10th of Production Ops (Per Instance).
 */

const char JOB_DATA_CPU[] =
    /* --- Stage 1 --- */
    "cpu       --ops 280    -c 1 --cpu-load 20\n"
    "matrix    --ops 640    -c 1 --matrix-size 32\n"
    "qsort     --ops 250    -c 1 --qsort-size 1024\n"
    "atomic    --ops 300000 -c 1 --atomic-thread 1\n"
    "bitops    --ops 2400   -c 1\n"
    "bsearch   --ops 310    -c 1 --bsearch-size 1024\n"
    "context   --ops 69000  -c 1 --context-threads 1\n"
    "fp        --ops 105    -c 1\n"
    "prime     --ops 270000 -c 1\n"
    "stack     --ops 690    -c 1 --stack-size 4096\n"
    "str       --ops 130000 -c 1 --str-size 1024\n"
    "trig      --ops 21     -c 1\n"
    "vecmath   --ops 270    -c 1\n"

    /* --- Stage 2 --- */
    "cpu       --ops 490    -c 2 --cpu-load 40\n"
    "matrix    --ops 280    -c 2 --matrix-size 64\n"
    "qsort     --ops 69     -c 2 --qsort-size 4096\n"
    "atomic    --ops 330000 -c 2 --atomic-thread 2\n"
    "bitops    --ops 2500   -c 2\n"
    "bsearch   --ops 120    -c 2 --bsearch-size 4096\n"
    "context   --ops 44000  -c 2 --context-threads 2\n"
    "fp        --ops 73     -c 2\n"
    "prime     --ops 210000 -c 2\n"
    "stack     --ops 670    -c 2 --stack-size 8192\n"
    "str       --ops 37500  -c 2 --str-size 4096\n"
    "trig      --ops 16     -c 2\n"
    "vecmath   --ops 190    -c 2\n"

    /* --- Stage 3 --- */
    "cpu       --ops 450    -c 3 --cpu-load 60\n"
    "matrix    --ops 160    -c 3 --matrix-size 64\n"
    "qsort     --ops 10     -c 3 --qsort-size 16384\n"
    "atomic    --ops 180000 -c 3 --atomic-thread 2\n"
    "bitops    --ops 1400   -c 3\n"
    "bsearch   --ops 14     -c 3 --bsearch-size 16384\n"
    "context   --ops 21000  -c 3 --context-threads 2\n"
    "fp        --ops 40     -c 3\n"
    "prime     --ops 130000 -c 3\n"
    "stack     --ops 370    -c 3 --stack-size 16384\n"
    "str       --ops 15500  -c 3 --str-size 8192\n"
    "trig      --ops 12     -c 3\n"
    "vecmath   --ops 130    -c 3\n"

    /* --- Stage 4 --- */
    "cpu       --ops 440    -c 4 --cpu-load 80\n"
    "matrix    --ops 28     -c 4 --matrix-size 128\n"
    "qsort     --ops 5      -c 4 --qsort-size 32768\n"
    "atomic    --ops 140000 -c 4 --atomic-thread 4\n"
    "bitops    --ops 1050   -c 4\n"
    "bsearch   --ops 5      -c 4 --bsearch-size 32768\n"
    "context   --ops 15000  -c 4 --context-threads 3\n"
    "fp        --ops 32     -c 4\n"
    "prime     --ops 110000 -c 4\n"
    "stack     --ops 270    -c 4 --stack-size 32768\n"
    "str       --ops 16500  -c 4 --str-size 8192\n"
    "trig      --ops 12     -c 4\n"
    "vecmath   --ops 135    -c 4\n"

    /* --- Stage 5 --- */
    "cpu       --ops 410    -c 5 --cpu-load 100\n"
    "matrix    --ops 19     -c 5 --matrix-size 128\n"
    "qsort     --ops 2      -c 5 --qsort-size 65536\n"
    "atomic    --ops 120000 -c 5 --atomic-thread 4\n"
    "bitops    --ops 880    -c 5\n"
    "bsearch   --ops 2      -c 5 --bsearch-size 65536\n"
    "context   --ops 13000  -c 5 --context-threads 3\n"
    "fp        --ops 26     -c 5\n"
    "prime     --ops 80000  -c 5\n"
    "stack     --ops 135    -c 5 --stack-size 65536\n"
    "str       --ops 2500   -c 5 --str-size 16384\n"
    "trig      --ops 10     -c 5\n"
    "vecmath   --ops 110    -c 5\n";

const char JOB_DATA_MEMORY[] =
    /* --- Stage 1 --- */
    "memcpy    --ops 1500    -c 1 --memcpy-size 32768\n"
    "stream    --ops 5200    -c 1 --stream-elem 2048\n"
    "vm        --ops 4200000 -c 1 --vm-bytes 1048576\n"
    "malloc    --ops 170000  -c 1 --malloc-bytes 256\n"
    "memthrash --ops 230     -c 1 --mem-size 65536\n"
    "ptr-chase --ops 8600000 -c 1 --ptr-chase-pages 128\n"

    /* --- Stage 2 --- */
    "memcpy    --ops 530     -c 2 --memcpy-size 65536\n"
    "stream    --ops 2050    -c 2 --stream-elem 4096\n"
    "vm        --ops 3750000 -c 2 --vm-bytes 4194304\n"
    "malloc    --ops 140000  -c 2 --malloc-bytes 1024\n"
    "memthrash --ops 220     -c 2 --mem-size 131072\n"
    "ptr-chase --ops 6200000 -c 2 --ptr-chase-pages 256\n"

    /* --- Stage 3 --- */
    "memcpy    --ops 230     -c 3 --memcpy-size 131072\n"
    "stream    --ops 900     -c 3 --stream-elem 8192\n"
    "vm        --ops 3900000 -c 2 --vm-bytes 2097152\n"
    "malloc    --ops 92000   -c 3 --malloc-bytes 4096\n"
    "memthrash --ops 180     -c 3 --mem-size 131072\n"
    "ptr-chase --ops 2750000 -c 3 --ptr-chase-pages 512\n"

    /* --- Stage 4 --- */
    "memcpy    --ops 70      -c 4 --memcpy-size 262144\n"
    "stream    --ops 270     -c 4 --stream-elem 16384\n"
    "vm        --ops 3750000 -c 2 --vm-bytes 4194304\n"
    "malloc    --ops 66000   -c 4 --malloc-bytes 8192\n"
    "memthrash --ops 110     -c 4 --mem-size 262144\n"
    "ptr-chase --ops 2000000 -c 4 --ptr-chase-pages 256\n"

    /* --- Stage 5 --- */
    "memcpy    --ops 30      -c 5 --memcpy-size 524288\n"
    "stream    --ops 110     -c 5 --stream-elem 32768\n"
    "vm        --ops 2900000 -c 3 --vm-bytes 8388608\n"
    "malloc    --ops 40000   -c 5 --malloc-bytes 16384\n"
    "memthrash --ops 90      -c 5 --mem-size 524288\n"
    "ptr-chase --ops 2600000 -c 5 --ptr-chase-pages 128\n";

const char JOB_DATA_FILE[] =
    /* --- Stage 1 --- */
    "hdd       --ops 42     -c 1 --hdd-bytes 131072\n"
    "open      --ops 250    -c 1 --open-max 32\n"
    "copy-file --ops 220    -c 1 --copy-file-bytes 131072\n"
    "unlink    --ops 250    -c 1\n"
    "fstat     --ops 600    -c 1 --fstat-files 10\n"
    "dentry    --ops 300    -c 1 --dentries 32\n"
    "rename    --ops 200    -c 1\n"
    "pipe      --ops 100    -c 1 --pipe-data-size 512\n"

    /* --- Stage 2 --- */
    "hdd       --ops 21     -c 2 --hdd-bytes 524288\n"
    "open      --ops 120    -c 2 --open-max 64\n"
    "copy-file --ops 82     -c 2 --copy-file-bytes 524288\n"
    "unlink    --ops 110    -c 2\n"
    "fstat     --ops 240    -c 2 --fstat-files 10\n"
    "dentry    --ops 130    -c 2 --dentries 64\n"
    "rename    --ops 100    -c 2\n"
    "pipe      --ops 20     -c 2 --pipe-data-size 4096\n"

    /* --- Stage 3 --- */
    "hdd       --ops 15     -c 3 --hdd-bytes 1048576\n"
    "open      --ops 80     -c 3 --open-max 50\n"
    "copy-file --ops 45     -c 3 --copy-file-bytes 1048576\n"
    "unlink    --ops 70     -c 3\n"
    "fstat     --ops 150    -c 3 --fstat-files 8\n"
    "dentry    --ops 80     -c 3 --dentries 128\n"
    "rename    --ops 70     -c 3\n"
    "pipe      --ops 30     -c 3 --pipe-data-size 32768\n"

    /* --- Stage 4 --- */
    "hdd       --ops 12     -c 4 --hdd-bytes 2097152\n"
    "open      --ops 80     -c 4 --open-max 40\n"
    "copy-file --ops 19     -c 4 --copy-file-bytes 2097152\n"
    "unlink    --ops 50     -c 4\n"
    "fstat     --ops 120    -c 4 --fstat-files 6\n"
    "dentry    --ops 60     -c 2 --dentries 128\n"
    "rename    --ops 35     -c 4\n"
    "pipe      --ops 30     -c 4 --pipe-data-size 32768\n"

    /* --- Stage 5 --- */
    "hdd       --ops 10     -c 5 --hdd-bytes 4194304\n"
    "open      --ops 70     -c 5 --open-max 40\n"
    "copy-file --ops 1      -c 5 --copy-file-bytes 4194304\n"
    "unlink    --ops 40     -c 5\n"
    "fstat     --ops 110    -c 5 --fstat-files 5\n"
    "dentry    --ops 60     -c 2 --dentries 128\n"
    "rename    --ops 30     -c 5\n"
    "pipe      --ops 30     -c 5 --pipe-data-size 32768\n";

#endif
