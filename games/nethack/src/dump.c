/*  SCCS Id: @(#)end.c  3.5 2010/03/08  */
/* Copyright (c) Patric Mueller.            */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "date.h"

#ifdef DUMP_LOG

# ifdef HAVE_UTIME_H
#  include <utime.h>
#  include <errno.h>
# endif

# ifdef UNIX /* DUMP-patch dump filename chmod() */
#  include <sys/types.h>
#  include <sys/stat.h>
# endif

extern char msgs[][BUFSZ];
extern int lastmsg;
void FDECL(do_vanquished, (int, BOOLEAN_P));
#endif

#ifdef DUMP_LOG
FILE *dump_fp = (FILE *)0;  /**< file pointer for text dumps */
char dump_path[BUFSIZ];
FILE *html_dump_fp = (FILE *)0;  /**< file pointer for html dumps */
char html_dump_path[BUFSIZ];
/* TODO:
 * - escape unmasked characters in html (done for map)
 * - started/ended date at the top
 */

static
char*
get_dump_filename()
{
    static char buf[BUFSIZ+1+5];
    char *f, *p, *end;
    int ispercent = 0;

    buf[0] = '\0';

    if (!dump_fn[0]) return NULL;

    f = dump_fn;
    p = buf;
    end = buf + sizeof(buf) - 10;

    while (*f) {
        if (ispercent) {
            switch (*f) {
            case 't':     /* starttime */
                snprintf (p, end + 1 - p, "%ld", u.ubirthday);
                while (*p != '\0') {
                    p++;
                }
                break;
            case 'N':     /* first character of player name */
                *p = plname[0];
                p++;
                *p = '\0';
                break;
            case 'n':     /* player name */
            case 's':     /* for backwards compatibility */
                snprintf(p, end + 1 - p, "%s", plname);
                while (*p != '\0') {
                    p++;
                }
                break;
            default:
                *p = *f;
                if (p < end) {
                    p++;
                }
            }
            ispercent = 0;
        } else {
            if (*f == '%') {
                ispercent = 1;
            } else {
                *p = *f;
                if (p < end) {
                    p++;
                }
            }
        }
        f++;
    }
    *p = '\0';

    return buf;
}

void
dump_init()
{
    if (dump_fn[0]) {
#ifdef UNIX
        mode_t dumpmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#endif
        char *new_dump_fn = get_dump_filename();

#ifdef DUMP_TEXT_LOG
        strncpy(dump_path, new_dump_fn, BUFSIZ-1);
        dump_fp = fopen(new_dump_fn, "w");
        if (!dump_fp) {
            pline("Can't open %s for output.", new_dump_fn);
            pline("Dump file not created.");
#ifdef UNIX
        } else {
            chmod(new_dump_fn, dumpmode);
#endif
        }
#endif
#ifdef DUMP_HTML_LOG
        strncpy(html_dump_path, strcat(new_dump_fn, ".html"), BUFSIZ-1);
        html_dump_fp = fopen(html_dump_path, "w");
        if (!html_dump_fp) {
            pline("Can't open %s for output.", new_dump_fn);
            pline("Html dump file not created.");
#ifdef UNIX
        } else {
            chmod(html_dump_path, dumpmode);
#endif
        }
#endif
    }
}
#endif

#ifdef DUMP_LOG
/** Set a file's access and modify time to u.udeathday. */
static void
adjust_file_timestamp(fpath)
const char* fpath;
{
# ifdef HAVE_UTIME_H
    if (u.udeathday > 0) {
        struct utimbuf tv;
        tv.actime = u.udeathday;
        tv.modtime = u.udeathday;
        if (utime(fpath, &tv)) {
            paniclog("adjust_file_timestamp: utime failed: ", strerror(errno));
        }
    }
# endif
}
#endif

void
dump_exit()
{
#ifdef DUMP_LOG
    if (dump_fp) {
        fclose(dump_fp);
        dump_fp = NULL;
        adjust_file_timestamp(dump_path);
    }
    if (html_dump_fp) {
        dump_html("</body>\n</html>\n", "");
        fclose(html_dump_fp);
        html_dump_fp = NULL;
        adjust_file_timestamp(html_dump_path);
    }
#endif
}

void
dump(pre, str)
const char *pre, *str;
{
#ifdef DUMP_LOG
    if (dump_fp)
        fprintf(dump_fp, "%s%s\n", pre, str);
    if (html_dump_fp)
        fprintf(html_dump_fp, "%s%s\n", pre, str);
#endif
}

/** Outputs a string only into the html dump. */
void
dump_html(format, str)
const char *format, *str;
{
#ifdef DUMP_LOG
    if (html_dump_fp)
        fprintf(html_dump_fp, format, str);
#endif
}

/** Outputs a string only into the text dump. */
void
dump_text(format, str)
const char *format, *str;
{
#ifdef DUMP_LOG
    if (dump_fp)
        fprintf(dump_fp, format, str);
#endif
}

/** Dumps one line as is. */
void
dump_line(pre, str)
const char *pre, *str;
{
#ifdef DUMP_LOG
    if (dump_fp)
        fprintf(dump_fp, "%s%s\n", pre, str);
    if (html_dump_fp) {
        while (*pre != '\0') {
            fprintf(html_dump_fp, "%s", html_escape_character(*pre++));
        }
        while (*str != '\0') {
            fprintf(html_dump_fp, "%s", html_escape_character(*str++));
        }
        fprintf(html_dump_fp, "<br />\n");
    }
#endif
}

extern boolean get_menu_coloring(const char *str, int *color, int *attr);

static char tmp_html_link[4*BUFSZ];
static char tmp_html_link_name[2*BUFSZ];
/** Return a link to nethackwiki . */
char *
html_link(link_name, name)
const char *link_name;
const char *name;
{
    tmp_html_link_name[0] = '\0';
    while (*name != '\0' && strlen(tmp_html_link_name) < BUFSZ - 10) {
        sprintf(eos(tmp_html_link_name), "%s", html_escape_character(*name++));
    }

    snprintf(tmp_html_link, sizeof(tmp_html_link),
             "<a href=\"http://nethackwiki.com/wiki/%s\">%s</a>",
             link_name, tmp_html_link_name);

    return tmp_html_link;
}

/** Dumps an object from the inventory. */
void
dump_object(c, obj, str)
const char c;
const struct obj *obj;
const char *str;
{
#ifdef DUMP_LOG
    const char *starting_inventory = obj->was_in_starting_inventory ? "*" : "";
    if (dump_fp)
        fprintf(dump_fp, "  %c - %s%s\n", c, str, starting_inventory);
    if (html_dump_fp) {
        char *link = html_link(dump_typename(obj->otyp), str);
# ifdef TTY_GRAPHICS
        int color;
        int attr;
#  ifdef MENU_COLOR
        if (iflags.use_menu_color &&
            get_menu_coloring(str, &color, &attr)) {
            fprintf(html_dump_fp, "<span class=\"nh_color_%d\"><span class=\"nh_item_letter\">%c</span> - %s</span>%s<br />\n", color, c, link, starting_inventory);
        } else
#  endif
# endif
        fprintf(html_dump_fp, "<span class=\"nh_item_letter\">%c</span> - %s%s<br />\n", c, link, starting_inventory);
    }
#endif
}

/** Dumps a secondary title. */
void
dump_subtitle(str)
const char *str;
{
#ifdef DUMP_LOG
    dump_text("  %s\n", str);
    dump_html("<h3>%s</h3>\n", str);
#endif
}

/** Dump a title. Strips : from the end of str. */
void
dump_title(str)
char *str;
{
#ifdef DUMP_LOG
    int len = strlen(str);
    if (str[len-1] == ':') {
        str[len-1] = '\0';
    }
    if (dump_fp)
        fprintf(dump_fp, "%s\n", str);
    if (html_dump_fp)
        fprintf(html_dump_fp, "<h2>%s</h2>\n", str);
#endif
}

/** Starts a list in the dump. */
void
dump_list_start()
{
#ifdef DUMP_LOG
    if (html_dump_fp)
        fprintf(html_dump_fp, "<ul>\n");
#endif
}

/** Dumps a linked list item. */
void
dump_list_item_link(link, str)
const char *link;
const char *str;
{
#ifdef DUMP_LOG
    if (dump_fp)
        fprintf(dump_fp, "  %s\n", str);
    if (html_dump_fp)
        fprintf(html_dump_fp, "<li>%s</li>\n", html_link(link, str));
#endif
}

/** Dumps an object as list item. */
void
dump_list_item_object(obj)
struct obj *obj;
{
#ifdef DUMP_LOG
    if (dump_fp)
        fprintf(dump_fp, "  %s\n", doname(obj));
    if (html_dump_fp) {
        const char* str = doname(obj);
        char *link = html_link(dump_typename(obj->otyp), str);
# ifdef TTY_GRAPHICS
        int color;
        int attr;
#  ifdef MENU_COLOR
        if (iflags.use_menu_color &&
            get_menu_coloring(str, &color, &attr)) {
            fprintf(html_dump_fp, "<li class=\"nh_color_%d\">%s</li>\n", color, link);
        } else
#  endif
# endif
        fprintf(html_dump_fp, "<li>%s</li>\n", link);
    }
#endif
}

/** Dumps a list item. */
void
dump_list_item(str)
const char *str;
{
#ifdef DUMP_LOG
    if (dump_fp)
        fprintf(dump_fp, "  %s\n", str);
    if (html_dump_fp)
        fprintf(html_dump_fp, "<li>%s</li>\n", str);
#endif
}

/** Ends a list in the dump. */
void
dump_list_end()
{
    dump_html("</ul>\n", "");
}

/** Starts a blockquote in the dump. */
void
dump_blockquote_start()
{
#ifdef DUMP_LOG
    if (html_dump_fp)
        fprintf(html_dump_fp, "<blockquote>\n");
#endif
}

/** Ends a blockquote in the dump. */
void
dump_blockquote_end()
{
#ifdef DUMP_LOG
    dump_text("\n", "");
    dump_html("</blockquote>\n", "");
#endif
}

/** Starts a definition list in the dump. */
void
dump_definition_list_start()
{
#ifdef DUMP_LOG
    if (html_dump_fp)
        fprintf(html_dump_fp, "<dl>\n");
#endif
}

/** Dumps a definition list item. */
void
dump_definition_list_dt(str)
const char *str;
{
#ifdef DUMP_LOG
    if (dump_fp)
        fprintf(dump_fp, "  %s\n", str);
    if (html_dump_fp) {
        fprintf(html_dump_fp, "<dt>");
        while (*str != '\0')
            fprintf(html_dump_fp, "%s", html_escape_character(*str++));
        fprintf(html_dump_fp, "</dt>\n");
    }
#endif
}

/** Dumps a definition list item. */
void
dump_definition_list_dd(str)
const char *str;
{
#ifdef DUMP_LOG
    if (dump_fp)
        fprintf(dump_fp, "  %s\n", str);
    if (html_dump_fp)
        fprintf(html_dump_fp, "<dd>%s</dd>\n", str);
#endif
}

/** Ends a list in the dump. */
void
dump_definition_list_end()
{
    dump_html("</dl>\n", "");
}

#ifdef DUMP_HTML_CSS_FILE
# ifdef DUMP_HTML_CSS_EMBEDDED
static
void
dump_html_css_file(const char *filename)
{
#  ifdef DUMP_HTML_LOG
    FILE *css = fopen(filename, "r");
    if (!css) {
        pline("Can't open %s for input.", filename);
        pline("CSS file not included.");
    } else if (css && html_dump_fp) {
        int c=0;
        while ((c=fgetc(css))!=EOF) {
            fputc(c, html_dump_fp);
        }
        fclose(css);
    }
#  endif
}
# endif
#endif


/** Dumps the HTML header. */
void
dump_header_html(title)
const char *title;
{
    dump_html("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n", "");
    dump_html("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n", "");
    dump_html("<head>\n", "");
    dump_html("<title>NetHack " VERSION_STRING ": %s</title>\n", title);
    dump_html("<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n", "");
    dump_html("<meta name=\"generator\" content=\"NetHack " VERSION_STRING "\" />\n", "");
    dump_html("<meta name=\"date\" content=\"%s\" />\n", iso8601(0));
#ifdef DUMP_HTML_CSS_FILE
# ifndef DUMP_HTML_CSS_EMBEDDED
    dump_html("<link rel=\"stylesheet\" type=\"text/css\" href=\"" DUMP_HTML_CSS_FILE "\" />\n", "");
# else
    dump_html("<style type=\"text/css\">\n", "");
    dump_html_css_file(DUMP_HTML_CSS_FILE);
    dump_html("</style>\n", "");
# endif
#endif
    dump_html("</head>\n", "");
    dump_html("<body>\n", "");
}

static char html_escape_buf[BUFSZ];
/** Escape a single character for HTML. */
const char* html_escape_character(const char c) {
    switch (c) {
    case '<':
        return "&lt;";
    case '>':
        return "&gt;";
    case '&':
        return "&amp;";
    case '\"':
        return "&quot;";
    case '\'':
        return "&#39;";     /* not &apos; */
    default:
        sprintf(html_escape_buf, "%c", c);
        return html_escape_buf;
    }
}

#ifdef DUMP_LOG
/** Screenshot of the HTML map. */
int
dump_screenshot()
{
    char screenshot[BUFSZ];
    char *filename = get_dump_filename();
    Sprintf(screenshot, "%s_screenshot_%ld_t%ld.html", filename, u.ubirthday, moves);

    html_dump_fp = fopen(screenshot, "w");
    if (!html_dump_fp) {
        pline("Can't open %s for output.", screenshot);
        pline("Screenshot file not created.");
    }

    dump_header_html("Screenshot");
    dump_screen();
    dump_exit();

    return 0;
}
#endif

/*dump.c*/
