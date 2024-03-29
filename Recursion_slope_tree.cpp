//=============================================================================
//                                INCLUDES                                    ;
//=============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>

#include "Enum.h"
#include "Stack.h"

//=============================================================================
//                               STRUCTURES                                   ;
//=============================================================================

struct func_t
{
    const char *name;
    const int num;
};

//-----------------------------------------------------------------------------

struct node_t
{
    int data;
    int type;
    node_t* left;
    node_t* right;
};

//-----------------------------------------------------------------------------

struct names_t
{
    char *name;
    int mean;
};

//=============================================================================
//                          GLOBAL/CONST VARIABLES                            ;
//=============================================================================

const int C_poison = -179179;

const int C_max_len = 40;

const int C_max_cnt_of_names = 10;

const int C_accuracy = pow (10, 3);

names_t* G_names_of_variables = nullptr;

const func_t C_functions[] = {
                            {"sin", E_sin},
                            {"cos", E_cos},
                            {"pow", E_pow},
                            {"dif", E_dif},
                            {"log", E_log},
                            {"exp", E_exp}
                             };

char *G_code = nullptr;

int G_cnt_of_labels = 10;

int G_cnt_of_labels_help = 5;

stack_t* G_stack_of_keywords_names = nullptr;

names_t* G_names_of_functions = nullptr;

int G_cnt_of_func_labels = 1;

int G_cnt_of_func_labels_help = 1;

names_t* G_arr_for_printf = nullptr;

int G_free_for_arr_for_printf = 0;

//=============================================================================
//                              HELPER FUNCTIONS                              ;
//=============================================================================

int Find_Sz_File (const char *fin);

bool Initialization ();

void Make_Right_Array (const char *helper, const int sz_file);

int Make_Op_For_Get_P_With_Key (const char chr1, const char chr2);

void Dump_Node (node_t* node);

void Skip_Empty_Lines ();

//=============================================================================
//                              RECURSION SLOPE                               ;
//=============================================================================

node_t* Create_Node (const int data, const int type);

//-----------------------------------------------------------------------------

node_t* Get_G ();

node_t* Get_Line ();

node_t* Get_E ();

node_t* Get_T ();

node_t* Get_P ();

node_t* Get_Id ();

node_t* Get_Assn (const int pos_of_arg);

node_t* Get_P_With_Key ();

node_t* Get_Label ();

node_t* Get_N ();

node_t* Get_P_With_Comma ();   // for functions with 2 arguments

//-----------------------------------------------------------------------------

void Syntax_Error (const char *name_of_func);

//-----------------------------------------------------------------------------

node_t* Case_Functions (const char *str);

node_t* Case_Keywords  (const char *str);

node_t* Case_Str_Is_Equal_If ();

node_t* Case_Str_Is_Equal_While ();

node_t* Case_Str_Is_Equal_Func ();

node_t* Case_Str_Is_Equal_Print ();

//=============================================================================
//                             PNG DUMP FOR TREE                              ;
//=============================================================================

void PNG_Dump (node_t* node);

void Print_PNG_Labels (node_t* node, FILE *fout);

void Print_PNG        (node_t* node, FILE *fout);

void Print_Node_Data_In_Right_Way (node_t* node, FILE *fout);

//=============================================================================
//                             ASM DUMP FOR TREE                              ;
//=============================================================================

void ASM_Dump (node_t* node);

void ASM_Dfs (node_t* node, FILE *fout);

void ASM_Make_Code (node_t* node, FILE *fout);

//=============================================================================
//                           MAKE CODE FROM TREE                              ;
//=============================================================================

void Make_Code_From_Tree (node_t* node);

void Dfs_For_Make_Code_From_Tree (node_t* node, FILE *fout);

void Print_In_Right_Form (node_t* node, FILE *fout);

void Print_In_Right_Form_For_E_op (node_t* node, FILE *fout);

void Print_In_Right_Form_For_E_key_op (node_t* node, FILE *fout);

//=============================================================================
//                              SIMPLIFY TREE                                 ;
//=============================================================================

node_t* Simplify_Tree (node_t* node);

node_t* Unit (node_t* node);

void Unit_Oper_With_Two_Args    (node_t* node);

bool Unit_For_One_And_Zero      (node_t* node);

void Unit_For_Oper_With_One_Arg (node_t* node);

node_t* Case_Differentiation (node_t* node);

node_t* Unit_Differentiation (node_t* node);

node_t* Unit_Copy (node_t* node_res, node_t* node_cpy);

//=============================================================================

int main ()
{
    Initialization ();

    node_t* node = Get_G ();

//    node = Simplify_Tree (node);
//    node = Simplify_Tree (node);
//    node = Simplify_Tree (node);

    PNG_Dump (node);
    ASM_Dump (node);
    Make_Code_From_Tree (node);

    printf ("END!\n");

    return 0;
}

//=============================================================================

int Find_Sz_File (const char *fin)
{
    struct stat text = {};

    stat (fin, &text);

    return text.st_size;
}

//-----------------------------------------------------------------------------

bool Initialization ()
{
    G_names_of_variables = (names_t *) calloc (C_max_cnt_of_names, sizeof (names_t *));
    G_names_of_functions = (names_t *) calloc (C_max_cnt_of_names, sizeof (names_t *));
    G_arr_for_printf     = (names_t *) calloc (C_max_cnt_of_names, sizeof (names_t *));

    for (int i = 0; i < C_max_cnt_of_names; i++)
    {
        G_names_of_variables[i].name = (char *) calloc (C_max_len, sizeof (char));
        G_names_of_variables[i].name = "";
        G_names_of_variables[i].mean = C_poison;

        G_names_of_functions[i].name = (char *) calloc (C_max_len, sizeof (char));
        G_names_of_functions[i].name = "";
        G_names_of_functions[i].mean = C_poison;

        G_arr_for_printf[i].name = (char *) calloc (C_max_len, sizeof (char));
        G_arr_for_printf[i].name = "";
        G_arr_for_printf[i].mean = C_poison;
    }

    const char name_of_file[] = "input.txt";

    int sz_file = Find_Sz_File (name_of_file);

    FILE* fin = fopen (name_of_file, "r");

    if (fin == nullptr) return false;

    G_code = (char *) calloc (sz_file + 2, sizeof (char));

    char *helper = (char *) calloc (sz_file + 2, sizeof (char));
    fread (helper, sizeof (char), sz_file, fin);

    Make_Right_Array (helper, sz_file);

    STACK_CONSTRUCTOR(G_stack_of_keywords_names)

    return true;
}

//-----------------------------------------------------------------------------

void Make_Right_Array (const char *helper, const int sz_file)
{
    int pos = 0;
    int cnt_of_not_space = 0;
    for (int i = 0; i < sz_file; i++)
    {
        if (helper[i] == EOF || (helper[i] == '\0' && i < sz_file))
        {
            ;
        }
        else if (!isspace (helper[i]))
        {
            G_code[pos++] = helper[i];
            cnt_of_not_space++;
        }
        else if (helper[i] == '\n')
        {
            if (cnt_of_not_space == 0)
            {
                G_code[pos++] = '$';
            }
            else
            {
                G_code[pos++] = helper[i];
            }

            cnt_of_not_space = 0;
        }
    }
}

//-----------------------------------------------------------------------------

int Make_Op_For_Get_P_With_Key (const char chr1, const char chr2)
{
    if (chr2 == '=')
    {
        G_code++;

        if (chr1 == '<')      return E_jae;
        else if (chr1 == '>') return E_jbe;
        else if (chr1 == '!') return E_jne;
        else if (chr1 == '=') return E_je;
    }
    else
    {
        if (chr1 == '<') return E_ja;

        return E_jb;
    }
}

//-----------------------------------------------------------------------------

void Dump_Node (node_t* node)
{
    printf ("NODE = %p\n", node);
    printf ("NODE->DATA = [%d]\n", node->data);
    printf ("NODE->TYPE = [%d]\n", node->type);
    printf ("NODE->LEFT  = %p\n", node->left);
    printf ("NODE->RIGHT = %p\n", node->right);

    printf ("\n");
}

//-----------------------------------------------------------------------------

void Skip_Empty_Lines ()
{
    while (*G_code == '\n' || *G_code == '$') G_code++;
}

//=============================================================================

node_t* Create_Node (const int data, const int type)
{
    node_t* node = (node_t *) calloc (1, sizeof (node_t) + 2);

    node->data  = data;
    node->type  = type;
    node->left  = nullptr;
    node->right = nullptr;

    if (node == nullptr)  Syntax_Error ("Create_Node nullptr\n");

    return node;
}

//=============================================================================

node_t* Get_G ()
{
    node_t* res = Get_Line ();

    if (*G_code == '\0')
        return res;
    else
    {
        Syntax_Error ("Get_G");
        printf ("G_code = (%c)\n", *G_code);
    }

    return nullptr;
}

//-----------------------------------------------------------------------------

node_t* Get_Line ()
{
    node_t* node1 = Get_E ();
    node_t* node2 = node1;
    node_t* res   = node1;

    int n_line = 1;

    while (*G_code == '\n' || *G_code == '$')
    {
        Skip_Empty_Lines ();

        if (*G_code == '\0' || *G_code == EOF)
        {
            return node1;
        }

        node2 = Get_E ();

        res = Create_Node (E_default, E_line); // $ may be not E_line (E_helper/E_pass)

        res->left  = node1;
        res->right = node2;

        node1 = res;
    }


    return res;
}

//-----------------------------------------------------------------------------

node_t* Get_E ()
{
    node_t* node1 = Get_T ();
    node_t* new_res = nullptr;

    while (*G_code == '+' || *G_code == '-')
    {
        char op = *G_code;

        G_code++;

        node_t* node2 = Get_T ();

        if (op == '+') new_res = Create_Node (E_plus,  E_op);
        else           new_res = Create_Node (E_minus, E_op);

        new_res->left  = node1;
        new_res->right = node2;

        node1 = new_res;
    }

    return node1;
}

//-----------------------------------------------------------------------------

node_t* Get_T ()
{
    node_t* node1 = Get_P ();

    while (*G_code == '*' || *G_code == '/')
    {
        char op = *G_code;
        G_code++;

        node_t* node2 = Get_P ();
        node_t* new_res = nullptr;

        if   (op == '*') new_res = Create_Node (E_mult, E_op);
        else             new_res = Create_Node (E_div , E_op);

        new_res->left  = node1;
        new_res->right = node2;

        node1 = new_res;
    }

    return node1;
}

//-----------------------------------------------------------------------------

node_t* Get_P ()
{
    if (*G_code == '(')
    {
        G_code++;
        node_t* helper = Get_E ();

        if (*G_code == ')')
        {
            G_code++;
            return helper;
        }
        else Syntax_Error ("Get_P\n");
    }
    else if ('a' <= *G_code && *G_code <= 'z')
    {
        node_t* res = Get_Id ();

        node_t* node = Get_Assn (res->data);
        if (node != nullptr) return node;

        return res;
    }
    else if (*G_code == ';')
    {
        G_code++;

        int helper = 0;
        Stack_Pop (G_stack_of_keywords_names, &helper);

        if (helper == E_if)
        {
            return Create_Node (E_end, E_key_op);
        }
        else if (helper == E_while)
        {
            return Create_Node (E_end, E_key_op);

        }
        else if (helper == E_func)
        {
            node_t* node = Create_Node (E_ret, E_key_op);
            return node;
        }
    }
    else
    {
        return Get_N ();
    }
}

//-----------------------------------------------------------------------------

node_t* Get_Id ()
{
    char *helper = (char *) calloc (C_max_len, sizeof (char));

    int pos = 0;
    helper[pos++] = *G_code;
    G_code++;

    while ('a' <= *G_code && *G_code <= 'z')
    {
        helper[pos++] = *G_code;
        G_code++;
    }

    node_t* node = nullptr;

    node = Case_Functions (helper);
    if (node != nullptr) return node;

    node = Case_Keywords (helper);
    if (node != nullptr) return node;

    static int cnt = 0;

    for (int i = 0; i < C_max_cnt_of_names; i++)
    {
        if (strcmp (G_names_of_variables[i].name, helper) == 0)
        {
            return Create_Node (i, E_str);
        }
    }

    G_names_of_variables[cnt++].name = helper;

    return Create_Node (cnt - 1, E_str);
}

//-----------------------------------------------------------------------------

node_t* Get_Assn (const int pos_of_arg)
{
    node_t* res = nullptr;

    if (*G_code == '=' && *(G_code + 1) != '=')
    {
        G_code++;

        res = Create_Node (E_equal, E_op);

        res->left = Create_Node (pos_of_arg, E_str);

        if ('a' <= *G_code && *G_code <= 'z')
        {
            char *helper = G_code;
            char *argument = (char *) calloc (C_max_len, sizeof (char));

            int pos = 0;
            while ('a' <= *G_code && *G_code <= 'z')
            {
                argument[pos++] = *G_code;
                G_code++;
            }

            bool ok = true;
            for (int i = 0; i < sizeof (C_functions) / sizeof (C_functions[0]); i++)
            {
                if (strcmp (C_functions[i].name, argument) == 0)
                {
                    ok = false;
                    break;
                }
            }

            G_code = helper;

            res->right = Get_E ();
        }
        else
        {
            res->right = Get_E ();
            G_names_of_variables[pos_of_arg].mean = (res->right)->data;
        }
    }

    return res;
}

//-----------------------------------------------------------------------------

node_t* Get_P_With_Key ()
{
    if (*G_code == '(')
    {
        G_code++;
        node_t* node1 = Get_E ();

        if (*G_code == '<'  || *G_code == '>'  || *G_code == '=' || *G_code == '!')
        {
            int op = Make_Op_For_Get_P_With_Key (*G_code, *(G_code + 1));

            G_code++;
            node_t* node2 = Get_E ();

            if (*G_code != ')')
            {
                Syntax_Error ("Get_P_With_Key have no ')'\n");
                return nullptr;
            }

            G_code++;

            node_t* node = Create_Node (op, E_key_op);
            node->left  = node1;
            node->right = node2;

            return node;
        }
        else Syntax_Error ("Get_P_With_Key have no right oper\n");
    }
    else Syntax_Error ("Get_P_With_Key have no '('\n");

    return nullptr;
}

//-----------------------------------------------------------------------------

node_t* Get_N ()
{
    int val = 0;

    val = val * 10 + (*G_code - '0');

    G_code++;

    while ('0' <= *G_code && *G_code <= '9')
    {
        val = val * 10 + (*G_code - '0');
        G_code++;
    }

    val *= C_accuracy;

    return Create_Node (val, E_int);
}

//-----------------------------------------------------------------------------

node_t* Get_P_With_Comma ()
{
    if (*G_code == '(')
    {
        G_code++;
        node_t* node1 = Get_E ();

        if (*G_code == ',')
        {
            G_code++;
            node_t* node2 = Get_E ();

            if (*G_code != ')')
            {
                Syntax_Error ("Get_P_With_Comma have no ')'\n");
                return nullptr;
            }

            G_code++;

            node_t* node = Create_Node (E_default, E_op);
            node->left  = node1;
            node->right = node2;

            return node;
        }
        else Syntax_Error ("Get_P_With_Comma have no ','\n");
    }
    else Syntax_Error ("Get_P_With_Comma have no '('\n");

    return nullptr;
}

//-----------------------------------------------------------------------------

void Syntax_Error (const char *name_of_func)
{
    printf ("SYNTAX ERROR IN %s!\n", name_of_func);
}

//=============================================================================

node_t* Case_Functions (const char *str)
{
    for (int i = 0; i < sizeof (C_functions) / sizeof (C_functions[0]); i++)
    {
        if (strcmp (C_functions[i].name, str) == 0)
        {
            node_t* node = Create_Node (C_functions[i].num, E_op);

            switch (C_functions[i].num)
            {
                case (E_default):
                {
                    printf ("OSHIBKA V INITIALIZATION, ERROR!!!\n");
                }

                case (E_sin):
                {
                    node->left = Get_P ();
                    break;
                }

                case (E_cos):
                {
                    node->left = Get_P ();
                    break;
                }

                case (E_pow):
                {
                    node = Get_P_With_Comma ();
                    node->data = E_pow;

                    break;
                }

                case (E_dif):
                {
                    node->left = Get_P ();
                    break;
                }

                case (E_log):
                {
                    node->left = Get_P ();
                    break;
                }

                case (E_exp):
                {
                    node->left = Get_P ();
                    break;
                }

                default:
                    printf ("NET TAKOY FUNC (Case_Functions), ERROR!!!\n");
            }

            return node;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------

node_t* Case_Keywords (const char *str)
{
    if (strcmp (str, C_keywords_names[E_if_ind]) == 0)
    {
        return Case_Str_Is_Equal_If ();
    }
    else if (strcmp (str, C_keywords_names[E_while_ind]) == 0)
    {
        return Case_Str_Is_Equal_While ();
    }
    else if (strcmp (str, C_keywords_names[E_func_ind]) == 0)
    {
        return Case_Str_Is_Equal_Func ();
    }
    else if (strcmp (str, C_keywords_names[E_ret_ind]) == 0)
    {
        return Create_Node (E_ret, E_key_op);
    }
    else if (strcmp (str, C_keywords_names[E_start_ind]) == 0)
    {
        return Create_Node (0, E_scope);
    }
    else if (strcmp (str, C_keywords_names[E_print_ind]) == 0)
    {
        return Case_Str_Is_Equal_Print ();
    }
    else if (strcmp (str, C_keywords_names[E_end_ind]) == 0)
    {
        return Create_Node (1, E_scope);
    }

    return nullptr;
}

//-----------------------------------------------------------------------------

node_t* Case_Str_Is_Equal_If ()
{
    Stack_Push (G_stack_of_keywords_names, E_if);

    node_t* node = Create_Node (E_if, E_key);

    node->left = Get_P_With_Key ();

    Skip_Empty_Lines ();

    node_t* node1 = Get_E ();

    Skip_Empty_Lines ();

    node_t* node2 = node1;
    node_t* res   = node1;

    while (node2->data != E_end && node2->type != E_key_op)
    {
        Skip_Empty_Lines ();

        node2 = Get_E ();

        if (node2->data == E_end && node2->type == E_key_op) break;

        res = Create_Node (E_default, E_line); // $ may be not E_line (E_helper/E_pass)

        res->left  = node1;
        res->right = node2;

        node1 = res;
    }

    node->right = res;

    return node;
}

//-----------------------------------------------------------------------------

node_t* Case_Str_Is_Equal_While ()
{
    Stack_Push (G_stack_of_keywords_names, E_while);

    node_t* node = Create_Node (E_while, E_key);

    node->left = Get_P_With_Key ();

    Skip_Empty_Lines ();

    node_t* node1 = Get_E ();

    Skip_Empty_Lines ();

    node_t* node2 = node1;
    node_t* res   = node1;

    while (node2->data != E_end && node2->type != E_key_op)
    {
        node2 = Get_E ();

        if (node2->data == E_end && node2->type == E_key_op) break;

        res = Create_Node (E_default, E_line); // $ may be not E_line (E_helper/E_pass)

        res->left  = node1;
        res->right = node2;

        node1 = res;
    }

    node->right = res;

    return node;
}

//-----------------------------------------------------------------------------

node_t* Case_Str_Is_Equal_Func ()
{
    char *helper = (char *) calloc (C_max_len, sizeof (char));

    if (*G_code != '_')
    {
        Syntax_Error ("NEED SPACE AFTER 'FUNC' !!!\n");
    }

    G_code++;

    int pos = 0;
    while ('a' <= *G_code && *G_code <= 'z')
    {
        helper[pos++] = *G_code;
        G_code++;
    }

    int free = 0;
    for (int i = 0; i < C_max_cnt_of_names; i++)
    {
        if (strcmp (helper, G_names_of_functions[i].name) == 0)
        {
            return Create_Node (G_names_of_functions[i].mean, E_call);   // call function
        }

        if (strcmp (G_names_of_functions[i].name, "") == 0) free = i;
    }

    G_names_of_functions[free].name = helper;
    G_names_of_functions[free].mean = G_cnt_of_func_labels++;

//        node_t* node = Create_Node (E_jmp, E_key_op);    // before function jump after (because we do not want to enter)
//        node->left = Create_Node (free, E_func_label);   // Label to call function

    Stack_Push (G_stack_of_keywords_names, E_func);

    return Create_Node (free, E_func_label);
}

//-----------------------------------------------------------------------------

node_t* Case_Str_Is_Equal_Print ()
{
    char *helper = (char *) calloc (C_max_len, sizeof (char));

    int pos = 0;
    int cnt_of_out = 0;

    while (*G_code != '!')
    {
        if (*G_code == '%') cnt_of_out++;

        helper[pos++] = *G_code;
        G_code++;
    }

    helper[pos++] = *G_code;
    G_code++;

//    printf ("HELPER = (%s)\n", helper);

    G_arr_for_printf[G_free_for_arr_for_printf].name = helper;

    node_t* node = Create_Node (G_free_for_arr_for_printf++, E_print);
    node_t* helper_node = node;

    for (int i = 0; i < cnt_of_out; i++)
    {
        helper_node->left = Create_Node (E_out, E_print);
        helper_node = helper_node->left;
    }

    return node;
}

//=============================================================================

void PNG_Dump (node_t* node)
{
    FILE *fout = fopen ("DOT", "w");

    fprintf (fout, "digraph\n");
    fprintf (fout, "{\n");
    fprintf (fout, "\"%p\" [label=\"", node);
    Print_Node_Data_In_Right_Way (node, fout);
    fprintf (fout, "\"]\n");

    Print_PNG_Labels (node, fout);
    Print_PNG        (node, fout);

    fprintf (fout, "}");

    fclose (fout);
}

//-----------------------------------------------------------------------------

void Print_PNG_Labels (node_t* node, FILE *fout)
{
    if (node -> left)
    {
        fprintf (fout, "\"%p\" [label=\"", node->left);
        Print_Node_Data_In_Right_Way (node->left, fout);
        fprintf (fout, "\"]\n");

        Print_PNG_Labels (node -> left, fout);
    }

    if (node -> right)
    {
        fprintf (fout, "\"%p\" [label=\"", node->right);
        Print_Node_Data_In_Right_Way (node->right, fout);
        fprintf (fout, "\"]\n");

        Print_PNG_Labels (node -> right, fout);
    }
}

//-----------------------------------------------------------------------------

void Print_PNG (node_t* node, FILE *fout)
{
    if (node -> left)
    {
        fprintf (fout, "\"%p\"->\"%p\";\n", node, node->left);
        Print_PNG (node -> left, fout);
    }

    if (node -> right)
    {
        fprintf (fout, "\"%p\"->\"%p\";\n", node, node->right);
        Print_PNG (node -> right, fout);
    }
}

//-----------------------------------------------------------------------------

void Print_Node_Data_In_Right_Way (node_t* node, FILE *fout)
{
    switch (node->type)
    {
        case (E_scope):
        {
            if (node->data == 0)
            {
                fprintf (fout, "START");
                return;
            }
            else if (node->data == 1)
            {
                fprintf (fout, "END");
                return;
            }
        }

        case (E_int):
        {
            fprintf (fout, "%lf", (double) (node->data) / C_accuracy);
            return;
        }

        case (E_str):
        {
            fprintf (fout, "%s", G_names_of_variables[node->data].name);
            return;
        }

        case (E_op):
        {
            Print_In_Right_Form_For_E_op (node, fout);
            return;
        }

        case (E_key):
        {
            switch (node->data)
            {
                case (E_if):
                {
                    fprintf (fout, "if");
                    return;
                }

                case (E_while):
                {
                    fprintf (fout, "while");
                    return;
                }

                case (E_func):
                {
                    fprintf (fout, "func");
                    return;
                }

                default:
                    printf ("NO E_KEY LIKE THIS!\n");
            }
        }

        case (E_key_op):
        {
            Print_In_Right_Form_For_E_key_op (node, fout);
            return;
        }

        case (E_line):
        {
            fprintf (fout, "LINE = [%d]\n", node->data);
            return;
        }

        case (E_call):
        {
            fprintf (fout, "CALL");
            return;
        }

        case (E_print):
        {
            if (node->data == E_out)
            {
                fprintf (fout, "OUT");
                return;
            }

            fprintf (fout, "%s", G_arr_for_printf[node->data].name);
            return;
        }
    }
}

//=============================================================================

void ASM_Dump (node_t* node)
{
    FILE *fout = fopen ("input_for_asm", "w");

    fprintf (fout, "JMP $0\n");

    ASM_Dfs (node, fout);

    fclose (fout);
}

//-----------------------------------------------------------------------------

void ASM_Dfs (node_t* node, FILE *fout)
{
    if (node->type == E_key_op && node->data == E_ret)
    {
        fprintf (fout, "RET\n");

        node->data = E_default;
    }

    if (node->type == E_op && node->data == E_equal)
    {
        if (node->right)
        {
            ASM_Dfs (node->right, fout);
        }
        else
        {
            printf ("NET PRAVOGO DITYA Y '=' ??? WUTFACE\n");
        }

        switch ((node->left)->data)
        {
            case (0):
            {
                fprintf (fout, "POP ax\n");
                return;
            }

            case (1):
            {
                fprintf (fout, "POP bx\n");
                return;
            }

            case (2):
            {
                fprintf (fout, "POP cx\n");
                return;
            }

            case (3):
            {
                fprintf (fout, "POP dx\n");
                return;
            }

            defualt:
                printf ("NE XVATAET REGISTROV!\n");
        }

    }
    else if (node->type == E_key)
    {
        if (node->data == E_if)
        {
            const int helper = G_cnt_of_labels;

            if (node->left)
            {
                ASM_Dfs (node->left, fout);
            }

            if (node->right)
            {
                ASM_Dfs (node->right, fout);
            }

            ASM_Make_Code (node, fout);

            fprintf (fout, "$%d\n", helper);
        }
        else if (node->data == E_while)
        {
            const int helper1 = G_cnt_of_labels;
            const int helper2 = G_cnt_of_labels_help;

            fprintf (fout, "$%d\n", G_cnt_of_labels_help++);

            if (node->left)
            {
                ASM_Dfs (node->left, fout);
            }

            if (node->right)
            {
                ASM_Dfs (node->right, fout);
            }

            ASM_Make_Code (node, fout);

            fprintf (fout, "JMP $%d\n", helper2);
            fprintf (fout, "$%d\n", helper1);
        }
    }
    else
    {
        if (node->left)
        {
            ASM_Dfs (node->left, fout);
        }

        if (node->right)
        {
            ASM_Dfs (node->right, fout);
        }

        ASM_Make_Code (node, fout);
    }
}

//-----------------------------------------------------------------------------

void ASM_Make_Code (node_t* node, FILE *fout)
{
    switch (node->type)
    {
        case (E_scope):
        {
            if (node->data == 0)
            {
                fprintf (fout, "$0\n");
                return;
            }
            else if (node->data == 1)
            {
                fprintf (fout, "END\n");
                return;
            }
        }

        case (E_int):
        {
            fprintf (fout, "PUSH %d\n", node->data / C_accuracy);
            return;
        }

        case (E_str):
        {
            switch (node->data)
            {
                case (0):
                {
                    fprintf (fout, "PUSH ax\n");
                    return;
                }

                case (1):
                {
                    fprintf (fout, "PUSH bx\n");
                    return;
                }

                case (2):
                {
                    fprintf (fout, "PUSH cx\n");
                    return;
                }

                case (3):
                {
                    fprintf (fout, "PUSH dx\n");
                    return;
                }

                defualt:
                    printf ("NET REGISTRA!\n");
                }
        }

        case (E_op):
        {
            switch (node->data)
            {
                case (E_default):
                {
                    printf ("DEFAULT???\n");
                    return;
                }

                case (E_plus):
                {
                    fprintf (fout, "ADD\n");
                    return;
                }

                case (E_minus):
                {
                    fprintf (fout, "SUB\n");
                    return;
                }

                case (E_mult):
                {
                    fprintf (fout, "MUL\n");
                    return;
                }

                case (E_div):
                {
                    fprintf (fout, "DIV\n");
                    return;
                }

                case (E_equal):
                {
                    fprintf (fout, "=\n");
                    return;
                }

                case (E_sin):
                {
                    fprintf (fout, "SIN\n");
                    return;
                }

                case (E_cos):
                {
                    fprintf (fout, "COS\n");
                    return;
                }

                case (E_pow):
                {
                    fprintf (fout, "POW\n");
                    return;
                }

                case (E_dif):
                {
                    fprintf (fout, "DIF\n");
                    return;
                }

                case (E_log):
                {
                    fprintf (fout, "LOG\n");
                    return;
                }

                case (E_exp):
                {
                    fprintf (fout, "EXP\n");
                    return;
                }

                default:
                    printf ("Make_Code -> ASM\n");
            }
        }

        case (E_key):
        {
            switch (node->data)
            {
                case (E_if):
                {
//                    fprintf (fout, "IF\n");
                    return;
                }

                case (E_while):
                {
//                    fprintf ("")
//                    fprintf (fout, "WHILE\n");
                    return;
                }

                case (E_func):
                {
//                    fprintf (fout, "FUNC\n");
                    return;
                }
            }
        }

        case (E_key_op):
        {
            switch (node->data)
            {
                case (E_ja):
                {
////                    fprintf (fout, "JA $%d\n", Stack_Top (G_labels));
//                    fprintf (fout, "JBE $%d\n", Stack_Top (G_labels));
                    fprintf (fout, "JBE $%d\n", G_cnt_of_labels++);
                    return;
                }

                case (E_jb):
                {
//////                    fprintf (fout, "JB $%d\n", Stack_Top (G_labels));
////                    fprintf (fout, "JAE $%d\n", Stack_Top (G_labels));
                    fprintf (fout, "JAE $%d\n", G_cnt_of_labels++);
                    return;
                }

                case (E_jae):
                {
////                    fprintf (fout, "JAE $%d\n", Stack_Top (G_labels));
//                    fprintf (fout, "JB $%d\n", Stack_Top (G_labels));
                    fprintf (fout, "JB $%d\n", G_cnt_of_labels++);
                    return;
                }

                case (E_jbe):
                {
////                    fprintf (fout, "JBE $%d\n", Stack_Top (G_labels));
//                    fprintf (fout, "JA $%d\n", Stack_Top (G_labels));
                    fprintf (fout, "JA $%d\n", G_cnt_of_labels++);
                    return;
                }

                case (E_jne):
                {
////                    fprintf (fout, "JNE $%d\n", Stack_Top (G_labels));
//                    fprintf (fout, "JE $%d\n", Stack_Top (G_labels));
                    fprintf (fout, "JE $%d\n", G_cnt_of_labels++);
                    return;
                }

                case (E_je):
                {
////                    fprintf (fout, "JE $%d\n", Stack_Top (G_labels));
//                    fprintf (fout, "JNE $%d\n", Stack_Top (G_labels));
                    fprintf (fout, "JNE $%d\n", G_cnt_of_labels++);
                    return;
                }

                case (E_jmp):
                {
                    fprintf (fout, "JMP $%d\n", G_cnt_of_labels++);
                    return;
                }

                case (E_ret):
                {
                    fprintf (fout, "RET\n");
                    return;
                }

                case (E_default):
                {
                    printf ("TUT!!\n");
                    return;
                }
            }
        }

        case (E_call):
        {
            fprintf (fout, "CALL $%d\n", node->data);
            return;
        }

        case (E_func_label):
        {
            fprintf (fout, "$%d\n", G_cnt_of_func_labels_help++);   // $
            return;
        }

        case (E_print):
        {
            if (node->data == E_out)
            {
                fprintf (fout, "OUT\n");
                return;
            }

            fprintf (fout, "PRT %s\n", G_arr_for_printf[node->data].name);
            return;
        }

        case (E_line):
        {
            return;
        }

        default:
            Dump_Node (node);
            printf ("TOP 10 OSHIBOK CHELOVECHESTVA (NEVOZMOZNO:D)\n");
    }
}

//=============================================================================

void Make_Code_From_Tree (node_t* node)
{
    FILE *fout = fopen ("make_code_back", "w");

    Dfs_For_Make_Code_From_Tree (node, fout);

    fclose (fout);
}

//-----------------------------------------------------------------------------

void Dfs_For_Make_Code_From_Tree (node_t* node, FILE *fout)
{
    if (node->type == E_key)
    {
        if (node->data == E_if)
        {
            fprintf (fout, "%s(", C_keywords_names[E_if_ind]);

            if (node->left)
            {
                Dfs_For_Make_Code_From_Tree (node->left, fout);
            }

            fprintf (fout, ")\n");

            if (node->right)
            {
                Dfs_For_Make_Code_From_Tree (node->right, fout);
            }

            fprintf (fout, "\n;\n");
        }
        else if (node->data == E_while)
        {
            fprintf (fout, "%s(", C_keywords_names[E_while_ind]);

            if (node->right)
            {
                Dfs_For_Make_Code_From_Tree (node->right, fout);
            }

            fprintf (fout, ")\n");
        }
    }
    else
    {
        int ok = 0;
        if (node->type == E_op && node->data != E_equal) ok = 1;

        if (ok) fprintf (fout, "(");

        if (node->left)
        {
            Dfs_For_Make_Code_From_Tree (node->left, fout);
        }

        Print_In_Right_Form (node, fout);

        if (node->right)
        {
            Dfs_For_Make_Code_From_Tree (node->right, fout);
        }

        if (ok) fprintf (fout, ")");
    }
}

//-----------------------------------------------------------------------------

void Print_In_Right_Form (node_t* node, FILE *fout)
{
    switch (node->type)
    {
        case (E_scope):
        {
            if (node->data == 0)
            {
                fprintf (fout, "%s\n", C_keywords_names[E_start_ind]);
                return;
            }
            else if (node->data == 1)
            {
                fprintf (fout, "%s\n", C_keywords_names[E_end_ind]);
                return;
            }
        }

        case (E_int):
        {
            fprintf (fout, "%.0lf", (double) node->data / C_accuracy); // $ may be another
            return;
        }

        case (E_str):
        {
            fprintf (fout, "%s", G_names_of_variables[node->data].name);
            return;
        }

        case (E_op):
        {
            Print_In_Right_Form_For_E_op (node, fout);
            return;
        }

        case (E_key):
        {
            return;
//            if (node->data == E_if)
//            {
//                return;
//            }
        }

        case (E_key_op):
        {
            Print_In_Right_Form_For_E_key_op (node, fout);
            return;
        }

        case (E_line):
        {
            fprintf (fout, "\n");
            return;
        }

        case (E_call):
        {
            for (int i = 0; i < C_max_cnt_of_names; i++)
            {
                if (G_names_of_functions[i].mean == node->data)
                {
                    fprintf (fout, "%s", G_names_of_functions[i].name);
                    return;
                }

                printf ("VERY STRANNO");
            }
        }

        case (E_print):
        {
            if (node->data == E_out)
            {
                fprintf (fout, "%");
                return;
            }

            fprintf (fout, "%s", C_keywords_names[E_print_ind]);
            fprintf (fout, "%s", G_arr_for_printf[node->data].name);
            return;
        }
    }
}

//-----------------------------------------------------------------------------

void Print_In_Right_Form_For_E_op (node_t* node, FILE *fout)
{
    switch (node->data)
    {
        case (E_plus):
        {
            fprintf (fout, "+");
            return;
        }

        case (E_minus):
        {
            fprintf (fout, "-");
            return;
        }

        case (E_mult):
        {
            fprintf (fout, "*");
            return;
        }

        case (E_div):
        {
            fprintf (fout, "/");
            return;
        }

        case (E_equal):
        {
            fprintf (fout, "=");
            return;
        }

        default:
            printf ("NET TAKOGO OPERATORA, ERROR!!!\n");
    }
}

//-----------------------------------------------------------------------------

void Print_In_Right_Form_For_E_key_op (node_t* node, FILE *fout)
{
    switch (node->data)
    {
        case (E_ja):
        {
            fprintf (fout, ">");
            return;
        }

        case (E_jb):
        {
            fprintf (fout, "<");
            return;
        }

        case (E_jae):
        {
            fprintf (fout, ">=");
            return;
        }

        case (E_jbe):
        {
            fprintf (fout, "<=");
            return;
        }

        case (E_jne):
        {
            fprintf (fout, "!=");
            return;
        }

        case (E_je):
        {
            fprintf (fout, "==");
            return;
        }

        case (E_jmp):
        {
            fprintf (fout, "JMP");
            return;
        }

        case (E_ret):
        {
            fprintf (fout, "RET");
            return;
        }

        case (E_end):
        {
            fprintf (fout, "END");
            return;
        }

        default:
        {
            printf ("NO E_KEY_OP LIKE THIS!\n");
            printf ("NODE->DATA = [%d]\n", node->data);
        }
    }
}

//=============================================================================

node_t* Simplify_Tree (node_t* node)
{
    if (node->left && ((node->left)->left || (node->left)->right))
    {
        node->left = Simplify_Tree (node->left);
    }

    if (node->right && ((node->right)->left || (node->right)->right))
    {
        node->right = Simplify_Tree (node->right);
    }

    return node;
}

//-----------------------------------------------------------------------------

node_t* Unit (node_t* node)
{
    if (node->type == E_key || node->type == E_key_op) return node;

    if (node->right)                // functions with 2 arguments
    {
        Unit_Oper_With_Two_Args (node);
    }
    else if (node->type == E_op)    // functions with 1 argument
    {
        Unit_For_Oper_With_One_Arg (node);
    }

    return node;
}

//-----------------------------------------------------------------------------

void Unit_Oper_With_Two_Args (node_t* node)
{
    if (Unit_For_One_And_Zero (node)) return;

    if (node->type == E_op && node->data == E_equal)
    {
        return;
    }

    int left  = C_poison;
    int right = C_poison;

    if ((node->left)->type == E_str && G_names_of_variables[(node->left)->data].mean != C_poison)
    {
        left = G_names_of_variables[(node->left)->data].mean;
    }
    else if ((node->left)->type == E_int)
    {
        left = (node->left)->data;
    }

    if ((node->right)->type == E_str && G_names_of_variables[(node->right)->data].mean != C_poison)
    {
        right = G_names_of_variables[(node->right)->data].mean;
    }
    else if ((node->right)->type == E_int)
    {
        right = (node->right)->data;
    }

    if (left != C_poison && right != C_poison)
    {
        switch (node->data)
        {
            case (E_default):
            {
                printf ("OSHIBKA PRI INITIALIZATION, ERROR!!!\n");
                return;
            }

            case (E_plus):
            {
                node->data = left + right;
                break;
            }

            case (E_minus):
            {
                node->data = left - right;
                break;
            }

            case (E_mult):
            {
                node->data = left * right / C_accuracy;
                break;
            }

            case (E_div):
            {
                node->data = left / right * C_accuracy;
                break;
            }

            case (E_pow):
            {
                node->data = (int) floor (C_accuracy * pow (left / C_accuracy, right / C_accuracy));
                break;
            }

            default:
                printf ("CHTO-TO NE TAK PRI PERENOSE(\n");
        }

        node->type = E_int;

        free (node->left);
        node->left = nullptr;
        free (node->right);
        node->right = nullptr;
    }
}

//-----------------------------------------------------------------------------

bool Unit_For_One_And_Zero (node_t* node)
{
    if (node->data == E_mult)            // case  *0 || 0* || 1* || *1
    {
        if (((node->left)->data == 0 && (node->left)->type  == E_int) ||                   // 0*
           ((node->right)->data == 0 && (node->right)->type == E_int))                     // *0
        {
            node->data = 0;
            node->type = E_int;

            free (node->left);
            node->left = nullptr;
            free (node->right);
            node->right = nullptr;

            return true;
        }
        else if (((node->left)->data == 1 * C_accuracy && (node->left)->type == E_int))    // 1*
        {
            free (node->left);

            node->data  = (node->right)->data;
            node->type  = (node->right)->type;
            node->right = nullptr;
            node->left  = nullptr;

            return true;
        }
        else if (((node->right)->data == 1 * C_accuracy && (node->right)->type == E_int))  // *1
        {
            free (node->right);

            node->data  = (node->left)->data;
            node->type  = (node->left)->type;
            node->right = nullptr;
            node->left  = nullptr;

            return true;
        }
    }
    else if (node->data == E_plus)       // case  +0 || 0+
    {
        if (((node->left)->data == 0 && (node->left)->type == E_int))
        {
            free (node->left);
            node->left = nullptr;

            node->data = node->right->data;
            node->type = node->right->type;

            free (node->right);
            node->right = nullptr;

            return true;
        }
        else if (((node->right)->data == 0 && (node->right)->type == E_int))
        {
            free (node->right);
            node->right = nullptr;

            node->data = node->left->data;
            node->type = node->left->type;

            free (node->left);
            node->left = nullptr;

            return true;
        }
    }
    else if (node->data == E_minus)      // case  -0
    {
        if (((node->right)->data == 0 && (node->right)->type == E_int))
        {
            free (node->right);
            node->right = nullptr;

            node->data = node->left->data;
            node->type = node->left->type;

            free (node->left);
            node->left = nullptr;

            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------

void Unit_For_Oper_With_One_Arg (node_t* node)
{
    if ((node->left)->type == E_int)
    {
        switch (node->data)
        {
            case (E_sin):
            {
                node->data = (int) floor (C_accuracy * sin ((node->left)->data / C_accuracy));
                break;
            }

            case (E_cos):
            {
                node->data = (int) floor (C_accuracy * cos ((node->left)->data / C_accuracy));
                break;
            }

            case (E_dif):
            {
                node = Case_Differentiation (node);
                break;
            }

            case (E_log):
            {
                node->data = (int) floor (C_accuracy * log ((node->left)->data / C_accuracy));
                break;
            }

            case (E_exp):
            {
                node->data = (int) floor (C_accuracy * exp ((node->left)->data / C_accuracy));
                break;
            }

            default:
                printf ("NET TAKOY FUNC (Unit), ERROR!!!\n");
        }

        node->type = E_int;

        free (node->left);
        node->left = nullptr;
    }
    else
    {
        if (node->data == E_dif)
        {
            node = Case_Differentiation (node);
        }
    }
}

//-----------------------------------------------------------------------------

node_t* Case_Differentiation (node_t* node)
{
    node_t* res = Unit_Differentiation (node->left);

    return res;
}

//-----------------------------------------------------------------------------

#define CN(data, type)\
    Create_Node (data, type)

#define UD(node)\
    Unit_Differentiation (node)

#define NL\
    node->left

#define NR\
    node->right

#define CPY(where, from)                 \
    where = CN(from->data, from->type);  \
    where = Unit_Copy (where, from);


node_t* Unit_Differentiation (node_t* node)
{
    if (node->type == E_int)
    {
        return CN(0, E_int);
    }

    if (node->type == E_str)
    {
        return CN(1 * C_accuracy, E_int);
    }

    if (node->type == E_op)
    {
        switch (node->data)
        {
            case (E_default):
            {
                printf ("OSHIBKA PRI INITIALIZATION, ERROR!!!\n");
                break;
            }

            case (E_plus):
            {
                node_t* res = CN(E_plus, E_op);
                res->left  = UD(NL);
                res->right = UD(NR);

                return res;
            }

            case (E_minus):
            {
                node_t* res = CN(E_minus, E_op);
                res->left  = UD(NL);
                res->right = UD(NR);

                return res;
            }

            case (E_mult):
            {
                node_t* res = CN(E_plus, E_op);

                res->left = CN(E_mult, E_op);
                (res->left)->left  = UD(NL);
                CPY((res->left)->right, NR)

                res->right = CN(E_mult, E_op);
                CPY((res->right)->left, NL)
                (res->right)->right = UD(NR);

                return res;
            }

            case (E_div):
            {
                node_t* res = CN(E_div, E_op);

                res->right = CN(E_pow, E_op);
                CPY((res->right)->left, NR)
                (res->right)->right = CN(2 * C_accuracy, E_int);


                (res->left) = CN(E_minus, E_op);

                (res->left)->left = CN(E_mult, E_op);     //}
                ((res->left)->left)->left  = UD(NL);      //|
                CPY(((res->left)->left)->right, NR)       //|
                                                          //|  mult, when res = res->left
                (res->left)->right = CN(E_mult, E_op);    //|
                CPY(((res->left)->right)->left, NL)       //|
                ((res->left)->right)->right = UD(NR);     //}

                return res;
            }

            case (E_sin):
            {
                node_t* res = CN(E_mult, E_op);

                res->left = CN(E_cos, E_op);
                CPY((res->left)->left, NL)

                res->right = UD(NL);

                return res;
            }

            case (E_cos):
            {
                node_t* res = CN(E_mult, E_op);

                res->left = CN(E_mult, E_op);
                (res->left)->left = CN(E_sin, E_op);
                CPY(((res->left)->left)->left, NL)

                (res->left)->right = CN(-1 * C_accuracy, E_int);

                res->right = UD(NL);

                return res;
            }

            case (E_pow):
            {
//                node_t* res = CN(E_dif, E_op);                   }
//                                                                 |
//                res->left         = CN(E_exp,  E_op);            |
//                (res->left)->left = CN(E_mult, E_op);            |
//                                                                 |
//                ((res->left)->left)->left = CN(E_log, E_op);     |  with exp
//                CPY((((res->left)->left)->left)->left, NL)       |
//                                                                 |
//                CPY(((res->left)->left)->right, NR)              |
//                                                                 |
//                return res;                                      }

                node_t* res = CN(E_mult, E_op);

                res->right = CN(E_pow, E_op);
                CPY((res->right)->left, NL)
                CPY((res->right)->right, NR)

                res->left = CN(E_dif, E_op);
                (res->left)->left = CN(E_mult, E_op);

                ((res->left)->left)->left = CN(E_log, E_op);
                CPY((((res->left)->left)->left)->left, NL)

                CPY(((res->left)->left)->right, NR)

                return res;
            }

            case (E_dif):
            {
                return Case_Differentiation (node);
            }

            case (E_log):
            {
                node_t* res = CN(E_mult, E_op);

                res->left = CN(E_div, E_op);
                (res->left)->left  = CN(1 * C_accuracy, E_int);
                CPY((res->left)->right, NL)

                res->right = CN(E_log, E_op);
                CPY((res->right)->left, NL)

                return res;
            }

            case (E_exp):
            {
                node_t* res = CN(E_mult, E_op);

                res->left = UD(NL);
                res->right = CN(E_exp, E_op);
                CPY((res->right)->left, NL);

                return res;
            }

            default:
                printf ("NET TAKOY FUNC, ERROR!!!\n");
        }
    }

    return nullptr;
}

#undef CN
#undef UF
#undef NL
#undef NR
#undef CPY

//-----------------------------------------------------------------------------

node_t* Unit_Copy (node_t* node_res, node_t* node_cpy)
{
    if (node_cpy->left)
    {
        node_res->left = Create_Node ((node_cpy->left)->data, (node_cpy->left)->type);
        Unit_Copy (node_res->left, node_cpy->left);
    }

    if (node_cpy->right)
    {
        node_res->right = Create_Node ((node_cpy->right)->data, (node_cpy->right)->type);
        Unit_Copy (node_res->right, node_cpy->right);
    }

    return node_res;
}
