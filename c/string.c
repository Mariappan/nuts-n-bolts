#include <stdio.h>
#include <string.h>

#define STR_LEN 5

static inline char *safestrncpy(
    char *dest,      /**< [out] destination string */
    const char *src, /**< [in] source string */
    size_t n         /**< [in] sizeof of destination string */)
{
    char *ret = strncpy(dest, src, n);
    dest[n-1] = 0;
    return ret;
}


void print_tree_name_sp(char *name)
{
    static char tree_prefix[] = "vlt_port_";
    static char tree_name[STR_LEN + sizeof(tree_prefix)];

    snprintf(tree_name, sizeof(tree_name), "%s%s", tree_prefix, name);
    printf("Tree name is %s. Pre(%lu/%lu) Name(%lu/%lu)\n", tree_name, sizeof tree_prefix, strlen(tree_prefix), sizeof tree_name, strlen(tree_name));
}

void print_tree_name(char *name)
{
    static char tree_prefix[] = "vlt_port_";
    static char tree_name[STR_LEN + sizeof(tree_prefix)];


    memset(tree_name, 0 , sizeof(tree_name));
    safestrncpy(tree_name, tree_prefix, sizeof(tree_name));
    strncat(tree_name, name, sizeof(tree_name)-strlen(tree_name));

    // printf("Tree name is %s. Pre(%lu/%lu) Name(%lu/%lu)\n", tree_name, sizeof tree_prefix, strlen(tree_prefix), sizeof tree_name, strlen(tree_name));
    tree_name[sizeof(tree_name)-1] = '\0';
    printf("Tree name is %s. Pre(%lu/%lu) Name(%lu/%lu)\n", tree_name, sizeof tree_prefix, strlen(tree_prefix), sizeof tree_name, strlen(tree_name));
}


int main()
{
    print_tree_name("m");
    print_tree_name("ma");
    print_tree_name("mar");
    print_tree_name("mario");
    print_tree_name("oriam");
    print_tree_name("mariams");
    print_tree_name("oriamszt");
    print_tree_name("mariamsar");

    print_tree_name_sp("m");
    print_tree_name_sp("ma");
    print_tree_name_sp("mar");
    print_tree_name_sp("mario");
    print_tree_name_sp("oriam");
    print_tree_name_sp("mariams");
    print_tree_name_sp("oriamszt");
    print_tree_name_sp("mariamsar");


    return 0;
}
