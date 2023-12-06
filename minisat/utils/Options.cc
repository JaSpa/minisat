/**************************************************************************************[Options.cc]
Copyright (c) 2008-2010, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include "minisat/mtl/Sort.h"
#include "minisat/utils/Options.h"
#include "minisat/utils/ParseUtils.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace Minisat {
void parseOptions(int& argc, char** argv, bool strict)
{
    int i, j;
    for (i = j = 1; i < argc; i++){
        const char* str = argv[i];
        if (match(str, "--") && match(str, Option::getHelpPrefixString()) && match(str, "help")){
            if (*str == '\0')
                printUsageAndExit(argc, argv);
            else if (match(str, "-verb"))
                printUsageAndExit(argc, argv, true);
        } else {
            bool parsed_ok = false;
        
            for (int k = 0; !parsed_ok && k < Option::getOptionList().size(); k++){
                parsed_ok = Option::getOptionList()[k]->parse(argv[i]);

                // fprintf(stderr, "checking %d: %s against flag <%s> (%s)\n", i, argv[i], Option::getOptionList()[k]->name, parsed_ok ? "ok" : "skip");
            }

            if (!parsed_ok){
                if (strict && match(argv[i], "-"))
                    fprintf(stderr, "ERROR! Unknown flag \"%s\". Use '--%shelp' for help.\n", argv[i], Option::getHelpPrefixString()), exit(1);
                else
                    argv[j++] = argv[i];
            }
        }
    }

    argc -= (i - j);
}


void setUsageHelp      (const char* str){ Option::getUsageString() = str; }
void setHelpPrefixStr  (const char* str){ Option::getHelpPrefixString() = str; }
void printUsageAndExit (int /*argc*/, char** argv, bool verbose)
{
    const char* usage = Option::getUsageString();
    if (usage != NULL)
        fprintf(stderr, usage, argv[0]);

    sort(Option::getOptionList(), Option::OptionLt());

    const char* prev_cat  = NULL;
    const char* prev_type = NULL;

    for (int i = 0; i < Option::getOptionList().size(); i++){
        const char* cat  = Option::getOptionList()[i]->category;
        const char* type = Option::getOptionList()[i]->type_name;

        if (cat != prev_cat)
            fprintf(stderr, "\n%s OPTIONS:\n\n", cat);
        else if (type != prev_type)
            fprintf(stderr, "\n");

        Option::getOptionList()[i]->help(verbose);

        prev_cat  = Option::getOptionList()[i]->category;
        prev_type = Option::getOptionList()[i]->type_name;
    }

    fprintf(stderr, "\nHELP OPTIONS:\n\n");
    fprintf(stderr, "  --%shelp        Print help message.\n", Option::getHelpPrefixString());
    fprintf(stderr, "  --%shelp-verb   Print verbose help message.\n", Option::getHelpPrefixString());
    fprintf(stderr, "\n");
    exit(0);
}

bool Option::parseEnv()
{
    std::string env_name{"MINISAT_"};
    for (const char *p = name; *p != '\0'; ++p)
        env_name.push_back(*p == '-' ? '_' : toupper(*p));

    const char *env_value = getenv(env_name.c_str());
    if (env_value && !parseValue(env_value, env_name.c_str(), /*strict=*/false)) {
        fprintf(stderr, "WARN! ignoring environment variable %s=%s", env_name.c_str(), env_value);
        return false;
    } else {
        return env_value;
    }
}

bool Option::parse(const char *str)
{
    const char *span = str;
    if (!match(span, "-") || !match(span, name) || !match(span, "="))
        return false;
    if (!parseValue(span))
        exit(1);
    return true;
}

DoubleOption::DoubleOption(const char *c, const char *n, const char *d, double def, DoubleRange r)
    : Option(n, d, c, "<double>"), range(r), value(def)
{
    (void)parseEnv();
}

bool DoubleOption::parseValue(const char *str, const char *env_name, bool strict)
{
    char *end;
    double tmp = strtod(str, &end);

    const char *prefix = strict ? "WARN" : "ERROR";
    if (end == nullptr || *end != '\0') {
        fprintf(stderr, "%s! value <%s> is invalid", prefix, str);
    } else if (tmp >= range.end && (!range.end_inclusive || tmp != range.end)) {
        fprintf(stderr, "%s! value <%s> is too large", prefix, str);
    } else if (tmp <= range.begin && (!range.begin_inclusive || tmp != range.begin)) {
        fprintf(stderr, "%s! value <%s> is too small", prefix, str);
    } else {
        // Value is ok!
        value = tmp;
        return true;
    }

    if (env_name) {
        fprintf(stderr, " for environment variable %s.\n", env_name);
    } else {
        fprintf(stderr, " for option \"%s\".\n", name);
    }
    return false;
}

void DoubleOption::help(bool verbose)
{
    fprintf(stderr, "  -%-12s = %-8s %c%4.2g .. %4.2g%c (default: %g)\n", name, type_name,
            range.begin_inclusive ? '[' : '(', range.begin, range.end, range.end_inclusive ? ']' : ')',
            value);
    if (verbose) {
        fprintf(stderr, "\n        %s\n", description);
        fprintf(stderr, "\n");
    }
}

IntOption::IntOption(const char *c, const char *n, const char *d, int32_t def, IntRange r)
    : Option(n, d, c, "<int32>"), range(r), value(def)
{
    (void)parseEnv();
}

bool IntOption::parseValue(const char *str, const char *env_name, bool strict)
{
    char *end;
    int32_t tmp = strtol(str, &end, 10);

    const char *prefix = strict ? "WARN" : "ERROR";
    if (end == nullptr || *end != '\0') {
        fprintf(stderr, "%s! value <%s> is invalid", prefix, str);
    } else if (tmp > range.end) {
        fprintf(stderr, "%s! value <%s> is too large", prefix, str);
    } else if (tmp < range.begin) {
        fprintf(stderr, "%s! value <%s> is too small", prefix, str);
    } else {
        // Value is ok!
        value = tmp;
        return true;
    }

    if (env_name) {
        fprintf(stderr, " for environment variable %s.\n", env_name);
    } else {
        fprintf(stderr, " for option \"%s\".\n", name);
    }
    return false;
}

void IntOption::help(bool verbose)
{
    fprintf(stderr, "  -%-12s = %-8s [", name, type_name);
    if (range.begin == INT32_MIN)
        fprintf(stderr, "imin");
    else
        fprintf(stderr, "%4d", range.begin);

    fprintf(stderr, " .. ");
    if (range.end == INT32_MAX)
        fprintf(stderr, "imax");
    else
        fprintf(stderr, "%4d", range.end);

    fprintf(stderr, "] (default: %d)\n", value);
    if (verbose) {
        fprintf(stderr, "\n        %s\n", description);
        fprintf(stderr, "\n");
    }
}

// Leave this out for visual C++ until Microsoft implements C99 and gets support for strtoll.
#ifndef _MSC_VER

Int64Option::Int64Option(const char *c, const char *n, const char *d, int64_t def, Int64Range r)
    : Option(n, d, c, "<int64>"), range(r), value(def)
{
    (void)parseEnv();
}

bool Int64Option::parseValue(const char *str, const char *env_name, bool strict)
{
    char *end;
    int64_t tmp = strtoll(str, &end, 10);

    const char *prefix = strict ? "WARN" : "ERROR";
    if (end == nullptr || *end != '\0') {
        fprintf(stderr, "%s! value <%s> is invalid", prefix, str);
    } else if (tmp > range.end) {
        fprintf(stderr, "%s! value <%s> is too large", prefix, str);
    } else if (tmp < range.begin) {
        fprintf(stderr, "%s! value <%s> is too small", prefix, str);
    } else {
        // Value is ok!
        value = tmp;
        return true;
    }

    if (env_name) {
        fprintf(stderr, " for environment variable %s.\n", env_name);
    } else {
        fprintf(stderr, " for option \"%s\".\n", name);
    }
    return false;
}

void Int64Option::help(bool verbose)
{
    fprintf(stderr, "  -%-12s = %-8s [", name, type_name);
    if (range.begin == INT64_MIN)
        fprintf(stderr, "imin");
    else
        fprintf(stderr, "%4" PRIi64, range.begin);

    fprintf(stderr, " .. ");
    if (range.end == INT64_MAX)
        fprintf(stderr, "imax");
    else
        fprintf(stderr, "%4" PRIi64, range.end);

    fprintf(stderr, "] (default: %" PRIi64 ")\n", value);
    if (verbose) {
        fprintf(stderr, "\n        %s\n", description);
        fprintf(stderr, "\n");
    }
}

#endif

StringOption::StringOption(const char *c, const char *n, const char *d, const char *def)
    : Option(n, d, c, "<string>"), value(def)
{
    (void)parseEnv();
}

bool StringOption::parseValue(const char *str, const char *env_name, bool strict)
{
    // Unused because parsing cannot fail.
    (void)env_name;
    (void)strict;

    // Value is taken literally.
    value = str;
    return true;
}

void StringOption::help(bool verbose)
{
    fprintf(stderr, "  -%-10s = %8s\n", name, type_name);
    if (verbose) {
        fprintf(stderr, "\n        %s\n", description);
        fprintf(stderr, "\n");
    }
}

BoolOption::BoolOption(const char *c, const char *n, const char *d, bool v)
    : Option(n, d, c, "<bool>"), value(v)
{
    parseEnv();
}

bool BoolOption::parse(const char *str)
{
    const char *span = str;

    if (!match(span, "-"))
        return false;

    // Option can have the form -no-<NAME> to disable.
    if (match(span, "no-")) {
        if (strcmp(span, name) == 0) {
            value = false;
            return true;
        } else {
            return false;
        }
    }

    if (!match(span, name))
        return false;

    // Option can have the form -<NAME>=<VALUE> with VALUE one of TRUE, YES, ON, 1 / FALSE, NO, OFF, 0.
    if (match(span, "=")) {
        if (!parseValue(span))
            exit(1);
        return true;
    }

    // Option can have the form -<NAME> to enable.
    if (*span == '\0') {
        value = true;
        return true;
    }

    return false;
}

bool BoolOption::parseValue(const char *str, const char *env_name, bool strict)
{
    // Lowercase string for case-insensitive comparison.
    std::string str_lower;
    for (const char *p = str; *p != '\0'; ++p)
        str_lower.push_back(tolower(*p));

    const char *true_values[] = {"true", "yes", "on", "1"};
    const char *false_values[] = {"false", "no", "off", "0"};
    if (std::find(std::begin(true_values), std::end(true_values), str_lower) != std::end(true_values)) {
        value = true;
        return true;
    }
    if (std::find(std::begin(false_values), std::end(false_values), str_lower) != std::end(false_values)) {
        value = false;
        return true;
    }

    fprintf(stderr, "%s! value <%s> is invalid for ", strict ? "ERROR" : "WARN", str);
    if (env_name)
        fprintf(stderr, "environment variable %s.\n", env_name);
    else
        fprintf(stderr, "option \"%s\".\n", name);
    return false;
}

void BoolOption::help(bool verbose)
{
    fprintf(stderr, "  -%s, -no-%s", name, name);

    for (uint32_t i = 0; i < 32 - strlen(name) * 2; i++)
        fprintf(stderr, " ");

    fprintf(stderr, " ");
    fprintf(stderr, "(default: %s)\n", value ? "on" : "off");
    if (verbose) {
        fprintf(stderr, "\n        %s\n", description);
        fprintf(stderr, "\n");
    }
}
} // namespace Minisat
