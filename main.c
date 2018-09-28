#include <unistd.h>
#include <stdio.h>
#include <limits.h>

#include <glib.h>
#include <glib/gprintf.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/object.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-gc.h>
#include <mono/utils/mono-publib.h>
#include <mono/metadata/loader.h>

#include <gladeui/glade.h>
#include <gtk/gtk.h>

#include "glade-custom-widget-editor.h"

#define _GNU_SOURCE
#include <dlfcn.h>

void mono_config_parse(char*);

static MonoDomain* domain=NULL;
static MonoImage* presentation_design_image=NULL;
static MonoImage* presentation_image=NULL;

#define LIBNAME "libglade_gtk-sharp_custom_widgets.so"
#define CONFIGNAME "libglade_gtk-sharp_custom_widgets.so.config"
#define WARNINGMESSAGE "GtkSharpCustomWidget-\033[1m\033[33mWarning\033[0m: "
#define INFOMESSAGE "GtkSharpCustomWidget-\033[1m\033[32mMessage\033[0m: "

typedef struct 
{
    const char *dli_fname;  /* Pathname of shared object that contains address */
    void       *dli_fbase;  /* Address at which shared object is loaded */
    const char *dli_sname;  /* Name of nearest symbol with address lower than addr */
    void       *dli_saddr;  /* Exact address of symbol named in dli_sname */
}Dl_info;

int dladdr(void *addr, Dl_info *info);

const char *my_fname(void)
{
    Dl_info dl_info;
    dladdr((void*)LIBNAME, &dl_info);
    return(dl_info.dli_fname);
}

char assembly_runtime_path[PATH_MAX];
char assembly_design_path[PATH_MAX];
char design_namespace[128];
char runtime_namespace[128];
char gtyperegister_static_class[128];
char gtyperegister_static_function[128];
GList* assembly_reference;

void register_types()
{
    MonoClass* class = mono_class_from_name(presentation_image, runtime_namespace, gtyperegister_static_class);
    MonoMethod* registerMethod = mono_class_get_method_from_name(class, gtyperegister_static_function, 0);
    mono_runtime_invoke(registerMethod, NULL, NULL, NULL);
}

void get_config_values()
{
    char conf_file[PATH_MAX];
    strcpy(conf_file, my_fname());
    strcat(conf_file, ".config");
    FILE* fp;
    char buffer[255];
    fp = fopen(conf_file, "rt");
    if (fp)
    {
#ifdef DEBUG
        printf(INFOMESSAGE "Configuration\n");
#endif
        int assembly_reference_count = 0;
        while(fgets(buffer, 255, (FILE*) fp)) 
        {
            if (strncmp(buffer, "runtime_namespace=", 18) == 0)
            {
                strncpy(runtime_namespace, &buffer[19], strlen(buffer)-21);
#ifdef DEBUG
                printf("\033[1m\033[32m\tfound key runtime_namespace\033[0m '%s'\n", runtime_namespace);
#endif
            }
            else if (strncmp(buffer, "gtyperegister_static_class=", 24) == 0)
            {
                strncpy(gtyperegister_static_class, &buffer[28], strlen(buffer)-30);
#ifdef DEBUG
                printf("\033[1m\033[32m\tfound key gtyperegister_static_class\033[0m '%s'\n", gtyperegister_static_class);
#endif
            }
            else if (strncmp(buffer, "gtyperegister_static_function=", 30) == 0)
            {
                strncpy(gtyperegister_static_function, &buffer[31], strlen(buffer)-33);
#ifdef DEBUG
                printf("\033[1m\033[32m\tfound key gtyperegister_static_function\033[0m '%s'\n", gtyperegister_static_function);
#endif
            }
            else if (strncmp(buffer, "assembly_runtime_path=", 22) == 0)
            {
                strncpy(assembly_runtime_path, &buffer[23], strlen(buffer)-25);
#ifdef DEBUG
                printf("\033[1m\033[32m\tfound key assembly_runtime_path\033[0m '%s'\n", assembly_runtime_path);
#endif
            }
            else if (strncmp(buffer, "assembly_design_path=", 21) == 0)
            {
                strncpy(assembly_design_path, &buffer[22], strlen(buffer)-24);
#ifdef DEBUG
                printf("\033[1m\033[32m\tfound key assembly_design_path\033[0m '%s'\n", assembly_design_path);
#endif
            }
            else if (strncmp(buffer, "design_namespace=", 17) == 0)
            {
                strncpy(design_namespace, &buffer[18], strlen(buffer)-20);
#ifdef DEBUG
                printf("\033[1m\033[32m\tfound key design_namespace\033[0m '%s'\n", design_namespace);
#endif
            }
            else if (strncmp(buffer, "assembly_reference", 18) == 0)
            {
                gpointer data = g_malloc(strlen(buffer)-22);
                memset(data, 0, strlen(buffer)-22);
                strncpy((char*)data, &buffer[21], strlen(buffer)-23);
                assembly_reference = g_list_append(assembly_reference, data);
#ifdef DEBUG
                printf("\033[1m\033[32m\tfound key assembly_reference\033[0m '%s'\n", data);
#endif
                assembly_reference_count++;
            }
        }
        fclose(fp);
    }
}

uint8_t types_registered;

void glade_gtksharp_custom_widgets_post_create(GladeWidgetAdaptor* adaptor, GObject* object, GladeCreateReason reason)
{
    if (domain != NULL || reason == GLADE_CREATE_REBUILD)
        return;

    assembly_runtime_path[0] = '\0';
    assembly_design_path[0] = '\0';
    design_namespace[0] = '\0';
    runtime_namespace[0] = '\0';
    gtyperegister_static_class[0] = '\0';
    gtyperegister_static_function[0] = '\0';
    assembly_reference = NULL;

    char my_name[PATH_MAX];
    strcpy(my_name, my_fname());
    char* p = strstr(my_name, LIBNAME);
    *p = 0;
#ifdef DEBUG
    mono_debug_init(MONO_DEBUG_FORMAT_MONO);
#endif

    mono_config_parse(NULL);
    mono_set_dirs(NULL, NULL);
    domain = mono_jit_init("Glade C# Custom Control Plugin");
    mono_domain_set_config(domain, my_name, CONFIGNAME);

    mono_thread_attach(domain);

    get_config_values();

    GList *tmpList;
    for (tmpList = assembly_reference; tmpList != NULL; tmpList = tmpList->next)
    {
        mono_domain_assembly_open(domain, (char*)tmpList->data);
    }
    g_list_free_full(assembly_reference, g_free);

    MonoAssembly* assembly = mono_domain_assembly_open(domain, assembly_runtime_path);
    if (assembly)
    {
        presentation_image = mono_assembly_get_image(assembly);
        assembly = mono_domain_assembly_open(domain, assembly_design_path);
        if (assembly)
        {
            presentation_design_image = mono_assembly_get_image(assembly);
            register_types();
        }
        else
            g_printf(WARNINGMESSAGE "C# Assembly '%s' not found\n", assembly_design_path);
    }
    else
        g_printf(WARNINGMESSAGE "C# Assembly '%s' not found\n", assembly_runtime_path);
}

GladeEditable* glade_gtksharp_custom_widgets_create_editable(GladeWidgetAdaptor* adaptor, GladeEditorPageType type)
{
    GladeEditable* editable;
    /* Get base editable */
    editable = GWA_GET_CLASS (GTK_TYPE_WIDGET)->create_editable (adaptor, type);
    if (type == GLADE_PAGE_GENERAL)
        return (GladeEditable*)glade_custom_widget_editor_new(adaptor, editable);
    return editable;
}

static void glade_custom_widget_editor_editable_init (GladeEditableIface* iface);
static void glade_custom_widget_editor_finalize (GObject* object);
static void glade_custom_widget_editor_grab_focus (GtkWidget* widget);
static GladeEditableIface* parent_editable_iface;

G_DEFINE_TYPE_WITH_CODE (GladeCustomWidgetEditor, glade_custom_widget_editor,
                         GTK_TYPE_BOX,
                         G_IMPLEMENT_INTERFACE (GLADE_TYPE_EDITABLE,
                                                glade_custom_widget_editor_editable_init));

static void glade_custom_widget_editor_class_init (GladeCustomWidgetEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = glade_custom_widget_editor_finalize;
  widget_class->grab_focus = glade_custom_widget_editor_grab_focus;
}

static void glade_custom_widget_editor_init (GladeCustomWidgetEditor *self)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
}

static void glade_custom_widget_editor_load (GladeEditable *editable, GladeWidget *widget)
{
    GladeCustomWidgetEditor *custom_widget_editor = GLADE_CUSTOM_WIDGET_EDITOR (editable);
    GList *l;

    /* Chain up to default implementation */
    parent_editable_iface->load (editable, widget);

    /* load the embedded editable... */
    if (custom_widget_editor->embed)
        glade_editable_load (GLADE_EDITABLE (custom_widget_editor->embed), widget);

    for (l = custom_widget_editor->properties; l; l = l->next)
        glade_editor_property_load_by_widget (GLADE_EDITOR_PROPERTY (l->data), widget);
}

static void glade_custom_widget_editor_set_show_name (GladeEditable *editable, gboolean show_name)
{
    GladeCustomWidgetEditor* glade_custom_widget = GLADE_CUSTOM_WIDGET_EDITOR (editable);
    glade_editable_set_show_name (GLADE_EDITABLE (glade_custom_widget->embed), show_name);
}

static void glade_custom_widget_editor_editable_init (GladeEditableIface *iface)
{
    parent_editable_iface = g_type_default_interface_peek (GLADE_TYPE_EDITABLE);
    iface->load = glade_custom_widget_editor_load;
    iface->set_show_name = glade_custom_widget_editor_set_show_name;
}

static void glade_custom_widget_editor_finalize (GObject *object)
{
    GladeCustomWidgetEditor* custom_widget_editor = GLADE_CUSTOM_WIDGET_EDITOR (object);
    if (custom_widget_editor->properties)
        g_list_free (custom_widget_editor->properties);
    custom_widget_editor->properties = NULL;
    custom_widget_editor->embed = NULL;
    glade_editable_load (GLADE_EDITABLE (object), NULL);
    G_OBJECT_CLASS (glade_custom_widget_editor_parent_class)->finalize (object);
}

static void glade_custom_widget_editor_grab_focus (GtkWidget *widget)
{
    GladeCustomWidgetEditor* custom_widget_editor = GLADE_CUSTOM_WIDGET_EDITOR (widget);
    gtk_widget_grab_focus (custom_widget_editor->embed);
}

GtkWidget* glade_custom_widget_editor_new (GladeWidgetAdaptor* adaptor, GladeEditable* editable)
{
    GladeCustomWidgetEditor* custom_widget_editor = g_object_new (GLADE_TYPE_CUSTOM_WIDGET_EDITOR, NULL);
    custom_widget_editor->embed = GTK_WIDGET(editable);
    gtk_box_pack_start(GTK_BOX(custom_widget_editor), GTK_WIDGET(editable), FALSE, FALSE, 0);

    char adaptor_name[128];
    strcpy(adaptor_name, glade_widget_adaptor_get_name(adaptor));
    char* tmp = strtok (adaptor_name, "_");
    char* class_name;
    while (tmp)
    {
        class_name = tmp;
        tmp = strtok (NULL, "_");
    }
    strcpy(adaptor_name, class_name);
    strcat(adaptor_name, "Editor");

    void* args[1];
    args[0] = &custom_widget_editor;
    MonoObject* exception = NULL;
    MonoClass* class = mono_class_from_name(presentation_design_image, design_namespace, adaptor_name);
    if (class)
    {
        MonoMethod* constructorMethod = mono_class_get_method_from_name(class, ".ctor", 1);
        if (constructorMethod)
        {
            MonoObject* instance = mono_object_new(domain, class);
            mono_runtime_invoke(constructorMethod, instance, args, &exception);
        }
        else
            g_printf(WARNINGMESSAGE "C# Constructor '%s' not found\n", adaptor_name);
    }
    else
        g_printf(WARNINGMESSAGE "C# Class '%s' not found\n", adaptor_name);
    gtk_widget_show_all (GTK_WIDGET (custom_widget_editor));
    return GTK_WIDGET (custom_widget_editor);
}