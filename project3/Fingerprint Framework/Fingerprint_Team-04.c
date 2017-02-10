/**
 * Team 04
 * Johann Beleites - 108015252567
 * Lion Hellstern - 108015263368
 *
 * Simple C-Tool that performes the simple matching algorithm. 
 * The -h flag can be used to toggle the use of the Hough-Transformation
 * The -p flag is used to determine a probe image in the xyt format
 * The -g flag is used to determine the gallery, either a single image
 *         or a complete directory with .xyt files
 * The -s flag can be used to test either a single file (when set) in this
 *         case the -g flag is the path of a single .xyt file, or (when not set)
         a whole directory. In this case the -g flag is the path to the dir. 
 * 
 * ************ ASSIGNMENT ***************
 * Implement the functions loadMinutiae, getScore and alignment.
 * The function documentation of the functions, and the exercise 
 * descriptions, contain more information !
 *
 * DO NOT CHANGE ANYTHING IN THE OTHER FUNCTIONS! 
 * BUT YOU ARE FREE TO ADD ADDITIONAL FUNCTIONS.
 * 
 * THE PROGRAMM HAS TO BE COMPILED WITH GCC ON A LINUX SYSTEM VIA:
 *         gcc -O2 simple_matcher.c -lm -o simple_matcher
 * IT CAN BE EXECUTED VIA:
 *         ./simple_matcher -p images/xxx_x.xyt -g images [-h]
 * ************ ASSIGNMENT ***************
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define MAX_MINUTIAE    130        /* should be ajusted if a file has more minutiae */
// Not required for our implementation: 
//#define A_X             400        /* used for Array in alignment, should be */
//#define A_Y             500        /* adjusted if out of boundaries error occurs*/
#define threshold_d      14        /* for getScore */
#define threshold_r      18        /* for getScore */
#define thres_t          18        /* for alignment */
#define PI                3.14159

struct xyt_struct {
    int nrows;
    int xcol[MAX_MINUTIAE];
    int ycol[MAX_MINUTIAE];
    int thetacol[MAX_MINUTIAE];
};

struct node {
    struct xyt_struct data;
    char* filepath;
    int score;
    struct node *next;
};

// Node struct to create a linked list for the sparse 3D-array
struct sa_node {
    struct sa_node *next;
    union Value {
        struct sa_node *branch;
        int a;
    } value;
    int dvalue;
};

void print_usage(char*);
void test_single(char*, char*, int);
void test_multiple(char*, char*, int);
struct xyt_struct alignment(struct xyt_struct probe, struct xyt_struct gallery);
struct xyt_struct loadMinutiae(const char *xyt_file);
int getScore(struct xyt_struct probe, struct xyt_struct gallery);
int inc_value_in_sa(struct sa_node*, int*, struct sa_node**);
struct sa_node* make_new_node(int, int);
void free_sa(struct sa_node*, int);

int main(int argc, char ** argv) {
    /* Shall Hough Transformation be used? */
    int hflag = 0;
    /* If set only one file is tested and galleryname is used a file name
       If not set, galleryname is used as a directory name and all files in
       this directory are compared to the probe image */
    int sflag = 0;
    /* String to the probe-image */
    char* probename = NULL;
    /* String to the gallery-image */
    char* galleryname = NULL;
    /* Name of this programm - needed for error msg */
    char* thisfile = argv[0];
    /* ARGUMENT PARSING - START */
    if(argc < 4) {
        print_usage(thisfile);
        exit(1);
    }
    int c = 0;
    opterr = 0;
    while ((c = getopt(argc, argv, "p:g:hs")) != -1) {
        switch (c) {
            case 'p':
                probename = optarg;
                break;
            case 'g':
                galleryname = optarg;
                break;
            case 'h':
                hflag = 1;
                break;
            case 's':
                sflag = 1;
                break;
            case '?':
                if(optopt == 'p' || optopt == 'g') {
                    printf("Opion -%c requires an argument!\n", optopt);
                    print_usage(thisfile);
                    return 1;
                }
            default:
                print_usage(thisfile);
                return 1;
        }
    }
    if(probename[0] == '-') {
        printf("Opion -p requires an argument!\n");
        print_usage(thisfile);
        return 1;
    }
    if(galleryname[0] == '-') {
        printf("Opion -g requires an argument!\n");
        print_usage(thisfile);
        return 1;
    }
    /* ARGUMENT PARSING - DONE */
    /* Test Minutiae, either single file or whole directory */
    if (sflag) {
        test_single(probename, galleryname, hflag);
    } else {
        test_multiple(probename, galleryname, hflag);
    }
    return 0;
}

void print_usage(char* thisfile) {
    printf("USAGE: %s -p probe-image -g gallery-image [-h] [-s]\n", thisfile);
    printf("\t -p \t The probe image that has to be tested!\n");
    printf("\t -g \t Path to galery images!\n");
    printf("\t[-h]\t If set the Hough Transformation is used!\n");
    printf("\t[-s]\t If set only a single file is tested and -g takes filename!\n");
    printf("\n");
}

void test_single(char* probename, char* galleryname, int hflag) {
    /* Load .xyt images to struct */
    struct xyt_struct probe = loadMinutiae(probename);
    struct xyt_struct gallery = loadMinutiae(galleryname);
    /* If Hough Should be used */
    if(hflag) {
        gallery = alignment(probe, gallery);
    }
    /* Calculate the score between the two .xyt images */
    int score = getScore(probe,gallery);    
    printf("The score is: %d\n",score);
}

void test_multiple(char* probename, char* dirname, int hflag) {
    /* Needed for dir scanning */
    DIR *d;
    struct dirent *dir;
    /* Needed for linked list of files */
    struct node *root;    //Root node
    struct node *curr;    //Current node
    /* To be loaded path of file is composed in it */
    char* toload = malloc(256 * sizeof(char));
    /* Create a new root node */
    root = (struct node *) malloc(sizeof(struct node));
    root->next = NULL;
    curr = root;
    printf("Looking for images in dir: %s\n", dirname);
    d = opendir(dirname);
    if(d) {
        while((dir = readdir(d)) != NULL) {
            //Leave out '.' and '..'
            if(dir->d_name[0] == '.') {
                continue;
            }
            int len = strlen(dir->d_name);
            //If the filename ends with xyt load the image to our list
            if(dir->d_name[len - 3] == 'x' && dir->d_name[len - 2] == 'y' && dir->d_name[len - 1] == 't') {
                //Compose the path of the to be loaded image in toload
                snprintf(toload, 256 * sizeof(char), "%s/%s", dirname, dir->d_name);
                //Store pathname in the struct
                int path_len = strlen(toload);
                curr->filepath = malloc(path_len * sizeof(char));
                strcpy(curr->filepath, toload);
                //Store loaded Minutiae in single linked list and add new next node
                curr->data = loadMinutiae(toload);
                curr->next = (struct node *) malloc(sizeof(struct node));
                curr = curr->next;
		curr->filepath = NULL; // Fix from 2017-01-19 by Marc Leuser
                //If the pointer returned by malloc was null we ran out of memory
                if(curr == NULL) {
                    printf("Out of memory!\n");
                    exit(1);
                }
                //Set next node to NULL so we have a sentinel at the end. 
                curr->next = NULL;
            }
        }
        //Close directory and free filepath composer when all files are loaded
        closedir(d);
        free(toload);
    }
    //Load the probe image
    struct xyt_struct probe = loadMinutiae(probename);
    //Set the current node back to root, so we can traverse the list again.
    curr = root;
    if(curr != NULL) {
        while(curr != NULL && curr->filepath != NULL) {
            /* If Hough Transformation shall be used */
            if(hflag) {
                curr->data = alignment(probe, curr->data);
            }
            curr->score = getScore(probe, curr->data);
            printf("Node: %p, Path: %s, Score: %d\n", curr, curr->filepath, curr->score);
            curr = curr->next;
        }
    } else {
        printf("The list is empty!\n");
        exit(1);
    }
}

/** 
 * The Generalised Hough Transform
 * 
 * The gallery image is aligned to the probe image. 
 * The function returns the aligned gallery xyt_struct.
 */
struct xyt_struct alignment(struct xyt_struct probe, struct xyt_struct galleryimage) {
    const int BUCKET_SIZE = 10;
    int i, j;
    // dvals[0] == dx, dvals[1] == dy, dvals[2] == dtheta.
    int dvals[3] = {0};

    /* 
     * Instead of using a regular c-array we use a sparse array instead.
     * In light of the relatively low number of minutiae we would be
     * storing a vast amount of zeros in a regular (3D) array, which is
     * quite wasteful of memory. Instead we use a datastructure consisting
     * of nested linked lists (over three levels), simulating a 3D-array.
     * This results in more required steps (worst-case O(n) for array size
     * n, mean is faster) to access an individual array entry but also
     * requires significantly less memory (only O(n) instead of what could
     * be O(n^3)). In the end this could even mean that we can keep more
     * relevant data cached, resulting in less main memory accesses, which
     * improves performance dramatically.
     *
     * We have also implemented a couple of helper functions for easier
     * handling of the sparse array. (All the while thinking how nicely one
     * could do this in a modern object-oriented programming language, such
     * as Java or Python). 
     */

    struct sa_node *root;
    root = NULL;
    int aVal, maxA = 0, max_xPos, max_yPos, max_tPos;

    // For all minutiae in the gallery image, compare them with all minutiae
    // of the probe image and increase the corresponding bucket's value.
    for(i = 0; i < galleryimage.nrows; i++) {
        for(j = 0; j < probe.nrows; j++) {
            dvals[2] = ((int) (probe.thetacol[j] - galleryimage.thetacol[i])) / BUCKET_SIZE;
            dvals[0] = ((int) (probe.xcol[j] - galleryimage.xcol[i] * cos(dvals[2] * PI / 180.0)
                               - galleryimage.ycol[i] * sin(dvals[2] * PI / 180.0))) / BUCKET_SIZE;
            dvals[1] = ((int) (probe.ycol[j] - galleryimage.ycol[i] * cos(dvals[2] * PI / 180.0)
                               + galleryimage.xcol[i] * sin(dvals[2] * PI / 180.0))) / BUCKET_SIZE;
            
            // We increase the corresponding A-array value
            aVal = inc_value_in_sa(root, dvals, &root);
            
            // In order to spare us from having to search the array another
            // time later on, we remember the maximum and the corresponding
            // position along the way.
            if(aVal > maxA) {
                maxA = aVal;
                max_xPos = dvals[0];
                max_yPos = dvals[1];
                max_tPos = dvals[2];
            }
        }
    }
    
    // All that is left to do is adjust the gallery image by the calculated
    // x, y and theta differences. Also Add half of BUCKET_SIZE to each value,
    // as we then get the average value by which we need to adjust the image,
    // if we assume that the values are distributed evenly along the range
    // of the bucket.
    for(i = 0; i < galleryimage.nrows; i++) {
        galleryimage.xcol[i] += BUCKET_SIZE * max_xPos + BUCKET_SIZE / 2;
        galleryimage.ycol[i] += BUCKET_SIZE * max_yPos + BUCKET_SIZE / 2;
        galleryimage.thetacol[i] += BUCKET_SIZE * max_tPos + BUCKET_SIZE / 2;
    }
    
    // Cleanup: free memory. Garbage collection is awesome.
    free_sa(root, 0);

    return galleryimage;
}

/**
 * This function will increment the A-array value in the sparse
 * array beginning with the root node pointed to by root_node.
 * The function creates new nodes as required.
 * dvals is an array of three integers, wihch contains dx, dy
 * and dt in that order.
 * The new_root argument is a return argument, which allows the
 * caller to obtain the new root pointer, in case the function
 * had to create a root.
 * The function returns the new A-array value after incrementing.
 */
int inc_value_in_sa(struct sa_node *root_node, int *dvals, struct sa_node **new_root) {
    int lvl = 0, i;
    struct sa_node *last_node, *cur_node;

    cur_node = root_node;
    last_node = cur_node;

    // Traverse the array until we either find the required leaf
    // node or we hit a point where the required next node does
    // not already exit.
    while(cur_node != NULL) {
        last_node = cur_node;
        if(cur_node->dvalue == dvals[lvl]) {
            // The required node on the current level was found.
            lvl++;
            if(lvl < 3) {
                // We have not found the correct leaf node yet
                // and need to dig deeper.
                cur_node = cur_node->value.branch;
            } else {
                // The correct leaf node exists and we have found it.
                break;
            }
        } else {
            // The current node is not the required node on the
            // current level. Hence, continue to its next neighbour.
            cur_node = cur_node->next;
        }
    }

    if(lvl < 3) {
        // We can simply generate the remaining path from this
        // point, as it does not exist. We have hit a NULL-
        // pointer earlier on.
        for(i = lvl; i < 3; i++) {
            // We create a new node. If we are at the lowest level
            // (2) we create a leaf node.
            cur_node = make_new_node(dvals[i], i == 2 ? 1 : 0);
            if(lvl == i) {
                if(last_node != NULL) {
                    last_node->next = cur_node;
                } else {
                    // If the last node is NULL it means that
                    // we are currently creating a new root node.
                    root_node = cur_node;
                }
            } else {
                last_node->value.branch = cur_node;
            }
            last_node = cur_node;
        }
    }

    // Finally increment the A-array value of the correct leaf node.
    cur_node->value.a++;

    // Set the return variable to the potentially new root node. This
    // usually does not do anything. Only when we create a new root
    // does this actually change something for the caller.
    *new_root = root_node;

    return cur_node->value.a;
}

/**
 * Creates new node for the sparse array. As first argument
 * it takes the d-value (so dx, dy or dt) to be associated
 * with the node and the second argument denotes whether the
 * node will be a leaf node (i.e. lowest level, where the
 * actual values of the A-matrix are stored).
 */
struct sa_node * make_new_node(int dval, int leaf_node) {
    struct sa_node* res;
    res = (struct sa_node*) malloc(sizeof(struct sa_node));
    res->dvalue = dval;
    res->next = NULL;

    if(leaf_node) {
        // If this is a leaf node, it will not have any
        // children. Instead, it will contain an actual vaule
        // from the A-array
        res->value.a = 0;
    } else {
        // If this is no leaf node, it will need to provide
        // a variable to point to the next-lower level, its
        // children.
        res->value.branch = NULL;
    }

    return res;
}

/**
 * And as we are using C we need to clean up our own mess.
 * An excellent chance to add some recursion to this program.
 */
void free_sa(struct sa_node *root, int lvl) {
    if(root->next != NULL) free_sa(root->next, lvl);
    if(lvl < 2 && root->value.branch != NULL)
        free_sa(root->value.branch, lvl + 1);
    free(root);
}

/**
 * The simple Minutiae Pairing Algorithm.
 * Compares the gallery image to the probe image and returns
 * the comparison score as an integer. 
 */
int getScore(struct xyt_struct probe, struct xyt_struct galleryimage) {
    int score=0;
    int usedG[galleryimage.nrows];
    int usedP[probe.nrows];
    int i, j, dd, dt, xtmp, ytmp, ttmp1, ttmp2;

    // Initialize used arrays to 0
    for(i = 0; i < galleryimage.nrows; i++) {
        usedG[i] = 0;
    }
    for(i = 0; i < probe.nrows; i++) {
        usedP[i] = 0;
    }

    // Do the actual score calculations
    for(i = 0; i < galleryimage.nrows; i++) {
        for(j = 0; j < probe.nrows; j++) {
            xtmp = galleryimage.xcol[i] - probe.xcol[j];
            ytmp = galleryimage.ycol[i] - probe.ycol[j];
            dd = (int) sqrt(xtmp * xtmp + ytmp * ytmp);

            // Account for large angles
            ttmp1 = abs(galleryimage.thetacol[i] - probe.thetacol[j]);
            ttmp2 = 360 - ttmp1;
            dt = ttmp1 < ttmp2 ? ttmp1 : ttmp2;
            
            if(usedG[i] == 0 && usedP[j] == 0 && dd < threshold_d && dt < threshold_r) {
                usedG[i] = 1;
                usedP[j] = 1;
                score++;
            }
        }
    }

    return score;
}

/** 
 * Loads minutiae from file (filepath is given as char *xyt_file) 
 * into the xyt_struct 'res' and returns it as the result.
 *
 * Checks for corrupted files, e.g. a line has less than 3, or more than 4
 * values, and fills the 'xcol', 'ycol' and 'thetacol' in the xyt_struct. 
 * At last sets the 'numofrows' variable in the xyt_struct to the amount of 
 * loaded rows.
 */
struct xyt_struct loadMinutiae(const char *xyt_file) {
    struct xyt_struct res;

    FILE *fp;
    // We open the file in read-only mode
    if(fp = fopen(xyt_file, "r")) {
        int i, r, x, y, theta, q;
        char ignore;

        // If a line is larger than 512 byte there is probably
        // something wrong anyway and we will discard the line.
        const int bufsize = 512;
        char lbuffer[bufsize];

        for(i = 0; i < MAX_MINUTIAE; i++) {
            if(fgets(lbuffer, bufsize, fp) != NULL) {
                r = sscanf(lbuffer, "%i %i %i %i %c", &x, &y, &theta, &q, &ignore);

                // Filter out all lines with too few/many values. r denotes
                // the number of values read from the line.
                if(r < 3 || r > 4) continue;
				
                // Store the data in the data structure
                res.nrows = i+1;
                res.xcol[i] = x;
                res.ycol[i] = y;
                res.thetacol[i] = theta;
            } else {
                // We have reached the EOF
                break;
            }
        }
		
        // Cleanup
        if(fp != NULL) fclose(fp);
    } else {
        fprintf(stderr, "Error opening file %s.\n", xyt_file);
    }

    return res;
}
