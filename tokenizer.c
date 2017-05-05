#include "tokenizer.h"

const int buffer_initial_capacity = 16;

void invite()
{
    printf("%s$ ", getenv("PWD"));
    fflush(stdout);
}

int initialize_buffer()
{
    buffer = malloc(sizeof(char) * buffer_initial_capacity);
    if (!buffer)
        return E_MEMFAIL;
    buffer_capacity = buffer_initial_capacity;
    buffer[0] = 0;
    buffer_position = 0;
    return E_OK;
}

void destroy_buffer()
{
    free(buffer);
}

int read_line_to_buffer()
{
    void *realloc_result;
    int c;

    errno = 0;
    buffer[0] = 0;
    buffer_position = 0;

    while (1)
    {
        c = getchar();
        if (c == EOF || c == '\n')
            break;

        if (!(buffer_position + 2 < buffer_capacity))
        {
            buffer_capacity <<= 1;
            realloc_result = realloc(buffer, buffer_capacity);
            if (realloc_result == NULL)
                return E_MEMFAIL;
            buffer = realloc_result;
        }

        buffer[buffer_position] = c;
        buffer[++buffer_position] = 0;
    }


    if (c == EOF && errno)
    {
        buffer_position = 0;
        return E_MEMFAIL;
    }

    buffer_position = 0;
    return (c == EOF ? E_EOF : E_OK);
}

void skip_whitespaces_and_tabs()
{
    while (buffer[buffer_position] == ' ' || buffer[buffer_position] == '\t')
        ++buffer_position;
    if (buffer[buffer_position] == '#')
        buffer[buffer_position] = 0;
}

int is_spec_symbol(char c)
{
    return (!c || c == ' ' || c == '\t' || c == '>' || c == '<' || c == '&' || c == '|' || c == ';');
}

/* status : 0 - no quotes, 1 - single quotes, 2 - double quotes */
int next_token1(char **res, char status, char *finish_status)
{
    char *result;
    size_t result_pos = 0;
    size_t result_capacity = 4;
    assert(status >= 0 && status <= 2);

    result = malloc(sizeof(char) * result_capacity);
    result[0] = 0;

    while (buffer[buffer_position] && (status != 0 || !is_spec_symbol(buffer[buffer_position])))
    {
        if (!(result_pos + 2 < result_capacity))
        {
            char *realloc_result;
            result_capacity <<= 1;
            realloc_result = realloc(result, result_capacity);
            if (!realloc_result)
            {
                free(result);
                *res = NULL;
                return E_MEMFAIL;
            }
            result = realloc_result;
        }

        if (buffer[buffer_position] == '\"')
        {
            if (status == 0)
            {
                status = 2;
            }
            else if (status == 1)
            {
                result[result_pos] = buffer[buffer_position];
                result[++result_pos] = 0;
            }
            else if (status == 2)
            {
                status = 0;
            }
        }
        else if (buffer[buffer_position] == '\'')
        {
            if (status == 0)
            {
                status = 1;
            }
            else if (status == 1)
            {
                status = 0;
            }
            else if (status == 2)
            {
                result[result_pos] = buffer[buffer_position];
                result[++result_pos] = 0;
            }
        }
        else if (buffer[buffer_position] == '\\' && status == 0)
        {
            result[result_pos] = buffer[++buffer_position];
            result[++result_pos] = 0;
        }
        else if (buffer[buffer_position] == '\\' && status == 1 && buffer[buffer_position + 1] == '\'')
        {
            result[result_pos] = buffer[++buffer_position];
            result[++result_pos] = 0;
        }
        else if (buffer[buffer_position] == '\\' && status == 2 && (buffer[buffer_position + 1] == '$' || buffer[buffer_position + 1] == '\"'))
        {
            result[result_pos] = buffer[++buffer_position];
            result[++result_pos] = 0;
        }
        else if (buffer[buffer_position] == '$' && status != 1)
        {
            char *to_be_appended = NULL;
            char need_free;

            ++buffer_position;
            if (buffer[buffer_position] >= '0' && buffer[buffer_position] <= '9')
            {
                int n = 0;
                while (buffer[buffer_position] >= '0' && buffer[buffer_position] <= '9')
                {
                    n *= 10;
                    n += buffer[buffer_position] - '0';
                    ++buffer_position;
                }
                --buffer_position;

                if (n >= argc_s)
                    to_be_appended = NULL;
                else
                    to_be_appended = argv_s[n];
                need_free = 0;
            }
            else if (buffer[buffer_position] == '#')
            {
                to_be_appended = malloc(sizeof(char) * 11);
                need_free = 1;
                if (!to_be_appended)
                {
                    free(result);
                    *res = NULL;
                    return E_MEMFAIL;
                }
                to_be_appended[0] = 0;
                sprintf(to_be_appended, "%d", argc_s);
            }
            else if (buffer[buffer_position] == '?')
            {
                to_be_appended = malloc(sizeof(char) * 11);
                need_free = 1;
                if (!to_be_appended)
                {
                    free(result);
                    *res = NULL;
                    return E_MEMFAIL;
                }
                to_be_appended[0] = 0;
                sprintf(to_be_appended, "%d", last_return_code);
            }
            else if (buffer[buffer_position] == '{')
            {
                char *env_name = NULL;
                size_t capacity = 8;
                size_t pos = 0;

                env_name = malloc(sizeof(char) * capacity);
                if (!env_name)
                {
                    free(result);
                    *res = NULL;
                    return E_MEMFAIL;
                }
                env_name[0] = 0;

                ++buffer_position;
                while (buffer[buffer_position] != '}')
                {
                    if (!buffer[buffer_position])
                    {
                        free(to_be_appended);
                        free(result);
                        *res = NULL;
                        return E_SYNTAX_ERROR;
                    }

                    if (!(pos + 2 < capacity))
                    {
                        char *env_name_realloc;

                        capacity <<= 1;
                        env_name_realloc = realloc(env_name, capacity);
                        if (!env_name_realloc)
                        {
                            free(env_name);
                            free(result);
                            *res = NULL;
                            return E_MEMFAIL;
                        }
                        env_name = env_name_realloc;
                    }

                    env_name[pos] = buffer[buffer_position];
                    env_name[++pos] = 0;
                    ++buffer_position;
                }

                to_be_appended = getenv(env_name);
                need_free = 0;

                free(env_name);
            }
            else
            {
                free(result);
                *res = NULL;
                return E_SYNTAX_ERROR;
            }

            if (to_be_appended)
            {
                while (!(result_pos + strlen(to_be_appended) + 4 < result_capacity))
                {
                    char *result_realloc;

                    result_capacity <<= 1;
                    result_realloc = realloc(result, result_capacity);
                    if (!result_capacity)
                    {
                        if (need_free)
                            free(to_be_appended);
                        free(result);
                        return E_MEMFAIL;
                    }
                    result = result_realloc;
                }

                strcat(result, to_be_appended);
                result_pos += strlen(to_be_appended);

                if (need_free)
                    free(to_be_appended);
            }

        }

        else if (status == 0 && buffer[buffer_position] == '#')
        {
            buffer[buffer_position] = 0;
            *res = result;
            *finish_status = status;
            return E_OK;
        }
        else
        {
            result[result_pos] = buffer[buffer_position];
            result[++result_pos] = 0;
        }

        if (!buffer[buffer_position]) {
            *res = result;
            *finish_status = status;
            return E_WRONG_QUOTE_BALANCE;
        }
        ++buffer_position;
    }

    *res = result;
    *finish_status = status;
    return status == 0 ? E_OK : E_WRONG_QUOTE_BALANCE;
}

int next_token2(char **res)
{
    char *accumulator;
    size_t accumulator_len;
    char *temp;
    size_t temp_len;

    char status = 0;
    int next_token1_res;
    char *realloc_result;

    next_token1_res = next_token1(&accumulator, status, &status);
    if (next_token1_res != E_OK && next_token1_res != E_WRONG_QUOTE_BALANCE)
    {
        *res = NULL;
        return next_token1_res;
    }
    accumulator_len = strlen(accumulator);

    while (next_token1_res != E_OK)
    {
        if (shell_isatty)
            printf("> ");
        fflush(stdout);
        read_line_to_buffer();

        next_token1_res = next_token1(&temp, status, &status);
        if (next_token1_res != E_OK && next_token1_res != E_WRONG_QUOTE_BALANCE)
        {
            free(accumulator);
            *res = NULL;
            return next_token1_res;
        }
        temp_len = strlen(temp);

        realloc_result = realloc(accumulator, accumulator_len + temp_len + 1);
        if (!realloc_result)
        {
            free(accumulator);
            free(temp);
            *res = NULL;
            return E_MEMFAIL;
        }
        accumulator = realloc_result;

        strcat(accumulator + accumulator_len, temp);
        free(temp);
        accumulator_len += temp_len;
    }

    *res = accumulator;
    return E_OK;
}

int next_program(program **res)
{
    int temp_res;
    program *p;

    skip_whitespaces_and_tabs();
    if (!buffer[buffer_position] || buffer[buffer_position] == '|' || buffer[buffer_position] == '&' || buffer[buffer_position] == '>' || buffer[buffer_position] == '<'
        || buffer[buffer_position] == ';')
    {
        *res = NULL;
        return E_SYNTAX_ERROR;
    }

    if (initialize_program(res) == E_MEMFAIL)
        return E_MEMFAIL;
    p = *res;

    p->number_of_arguments = 1;
    p->arguments = malloc(sizeof(char*) * p->number_of_arguments);
    if (!p->arguments)
    {
        destroy_program(res);
        return E_MEMFAIL;
    }

    temp_res = next_token2(&p->arguments[0]);
    if (temp_res != E_OK)
    {
        destroy_program(res);
        return temp_res;
    }

    while (1)
    {
        skip_whitespaces_and_tabs();
        if (!buffer[buffer_position] || buffer[buffer_position] == ';' || buffer[buffer_position] == '|' || buffer[buffer_position] == '&')
        {
            break;
        }

        if (buffer[buffer_position] == '>' || buffer[buffer_position] == '<')
        {
            if (buffer[buffer_position] == '>')
            {
                free(p->output_file);

                if (buffer[buffer_position + 1] == '>')
                    p->output_type = 2;
                else
                    p->output_type = 1;

                ++buffer_position;
                if (p->output_type == 2)
                    ++buffer_position;
                skip_whitespaces_and_tabs();
                if (!buffer[buffer_position]
                    || buffer[buffer_position] == ';'
                    || buffer[buffer_position] == '|'
                    || buffer[buffer_position] == '>'
                    || buffer[buffer_position] == '<')
                {
                    destroy_program(res);
                    return E_SYNTAX_ERROR;
                }

                temp_res = next_token2(&p->output_file);
                if (temp_res != E_OK)
                {
                    destroy_program(res);
                    return temp_res;
                }
            }
            else
            {
                free(p->input_file);

                ++buffer_position;
                skip_whitespaces_and_tabs();
                if (!buffer[buffer_position]
                    || buffer[buffer_position] == ';'
                    || buffer[buffer_position] == '|'
                    || buffer[buffer_position] == '>'
                    || buffer[buffer_position] == '<')
                {
                    destroy_program(res);
                    return E_SYNTAX_ERROR;
                }

                temp_res = next_token2(&p->input_file);
                if (temp_res != E_OK)
                {
                    destroy_program(res);
                    return temp_res;
                }
            }
        }
        else
        {
            char **realloc_result;

            p->number_of_arguments += 1;
            realloc_result = realloc(p->arguments, sizeof(char*) * p->number_of_arguments);
            if (!realloc_result)
            {
                p->number_of_arguments -= 1;
                destroy_program(res);
                return E_MEMFAIL;
            }
            p->arguments = realloc_result;

            temp_res = next_token2(&p->arguments[p->number_of_arguments - 1]);
            if (temp_res != E_OK)
            {
                destroy_program(res);
                return temp_res;
            }
        }
    }

    {
        char **realloc_result = realloc(p->arguments, sizeof(char *) * (p->number_of_arguments + 1));
        if (!realloc_result)
        {
            destroy_program(res);
            return E_MEMFAIL;
        }
        p->arguments = realloc_result;
        p->arguments[p->number_of_arguments] = NULL; /* requirement of exec */
    }

    return E_OK;
}

int next_job(job **res)
{
    job *j;

    if (initialize_job(res) == E_MEMFAIL)
        return E_MEMFAIL;
    j = *res;

    while (1)
    {
        skip_whitespaces_and_tabs();

        if (buffer[buffer_position] == ';' || !buffer[buffer_position])
            break;

        if (buffer[buffer_position] == '&')
        {
            ++buffer_position;
            skip_whitespaces_and_tabs();
            if (!(buffer[buffer_position] == 0 || buffer[buffer_position] == ';') || !j->number_of_programs)
            {
                destroy_job(res);
                return E_SYNTAX_ERROR;
            }
            j->background = 1;
            break;
        }

        if (buffer[buffer_position] == '|')
        {
            ++buffer_position;
            skip_whitespaces_and_tabs();
            if (is_spec_symbol(buffer[buffer_position]))
            {
                destroy_job(res);
                return E_SYNTAX_ERROR;
            }
            continue;
        }

        if (is_spec_symbol(buffer[buffer_position]))
        {
            destroy_job(res);
            return E_SYNTAX_ERROR;
        }

        j->number_of_programs += 1;
        {
            program **realloc_result = realloc(j->programs, sizeof(program*) * j->number_of_programs);
            if (!realloc_result)
            {
                j->number_of_programs -= 1;
                destroy_job(res);
                return E_MEMFAIL;
            }
            j->programs = realloc_result;
        }

        {
            int next_program_res;

            next_program_res = next_program(&j->programs[j->number_of_programs - 1]);
            if (next_program_res != E_OK)
            {
                destroy_job(res);
                return next_program_res;
            }
        }
    }

    return E_OK;
}

void print_program(FILE *stream, const program *p)
{
    int i;

    if (!p->number_of_arguments)
        return;

    for (i = 0; i < p->number_of_arguments - 1; ++i)
        fprintf(stream, "%s ", p->arguments[i]);
    fprintf(stream, "%s", p->arguments[p->number_of_arguments - 1]);
    if (p->input_file)
        fprintf(stream, " <%s", p->input_file);
    if (p->output_file)
    {
        fprintf(stream, (p->output_type == 1 ? " >" : " >>"));
        fprintf(stream, "%s", p->output_file);
    }
    fflush(stream);
}

void print_job(FILE *stream, const job *j)
{
    int i;

    if (!j->number_of_programs)
        return;

    for (i = 0; i < j->number_of_programs - 1; ++i)
    {
        print_program(stream, j->programs[i]);
        fprintf(stream, " | ");
    }
    print_program(stream, j->programs[j->number_of_programs - 1]);
}

int check_built_in_piped(const job *j)
{
    int i;

    assert(j->number_of_programs > 1);

    for (i = 0; i < j->number_of_programs; ++i)
        if (!strcmp(j->programs[i]->arguments[0], "cd")
            || !strcmp(j->programs[i]->arguments[0], "exit")
            || !strcmp(j->programs[i]->arguments[0], "fg")
            || !strcmp(j->programs[i]->arguments[0], "bg")
            || !strcmp(j->programs[i]->arguments[0], "jobs")
                )
            return E_BUILT_IN_PIPED;
    return E_OK;
}
