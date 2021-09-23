#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <libxml/HTMLparser.h>

#ifdef TIDY
#include <tidy.h>
#include <tidybuffio.h>
#endif

#include "library.h"

//Image_data *tex;
//const char ut[] = u8"This is a unicode string : ééé";
//printf("%s \n", ut);

/* This should probably be moved elsewhere */
int video_width = 640;
int video_height = 480;
int bitdepth = 16;

int counter = 0;
int text_color;
int global_background_color = 0;

struct text_html
{
	int text_order;
	char* text_to_hold;
	int type;
	int status;
	int used;
};

struct image_html
{
	int status;
	char* filename;
	int loaded;
	int width;
	int height;
	int used;
	Image_data* texture;
};

struct css_html
{
	int status;
	char* filename;
	int loaded;
	int width;
	int height;
	int used;
	int color;
};

struct status_toprint_html
{
	int status;
	int type;
	int number;
	int used;
};

struct text_html text_toprint[500];
struct image_html img_toprint[500];

enum
{
	STATUS_TEXT_TYPE = 1,
	STATUS_IMAGE_TYPE = 2,
};

struct status_toprint_html status_toprint[2000];

int ishtml_valid = 0;

int last_set_type = 0;
int attb_set_type = 0;
size_t last_size_content = 0;

int number_of_files_print = 0;
int number_of_p_print = 0;
int number_of_img_print = 0;

char* title_html;

enum
{
	HTML_TAG = 0,
	TITLE_TAG = 1,
	HEADER_TAG = 2,
	META_TAG = 3,
	STYLE_TAG = 4,
	SCRIPT_TAG = 5,
	BODY_TAG = 6,
	HEAD_TAG = 7,
	IMG_TAG = 8,
	VIDEO_TAG = 9,
	P_TAG = 10
};

const char available_tags[11][10] =
{
	"html",
	"title",
	"header",
	"meta",
	"style",
	"script",
	"body",
	"head",
	"img",
	"video",
	"p"
};

enum
{
	IMG_HREF_TAG = 0,
	IMG_SRC_TAG = 1,
	IMG_ALT_TAG = 2,
	IMG_WIDTH_TAG = 3,
	IMG_HEIGHT_TAG = 4
};

const char img_tags[5][10] =
{
	"href",
	"src",
	"alt",
	"width",
	"height"
};

enum
{
	CSS_TAG_HTML = 0,
	CSS_TAG_BODY = 1,
	CSS_TAG_ROOT = 2,
	CSS_TAG_HEAD = 3,
	CSS_TAG_TABLE = 4,
	CSS_TAG_UL = 5,
	CSS_TAG_LI = 6,
	CSS_TAG_CANVAS = 7,
	CSS_TAG_IMG = 8,
	CSS_TAG_VIDEO = 9
};

const char css_tags_type_list[10][10] =
{
	"html",
	"body",
	":root",
	"head",
	"table",
	"ul",
	"li",
	"canvas",
	"img",
	"video"
};

enum
{
	CSS_TAG_ATT_BACKGROUND_COLOR = 0,
	CSS_TAG_ATT_COLOR = 1,
	CSS_TAG_ATT_WIDTH = 2,
	CSS_TAG_ATT_HEIGHT = 3,
	CSS_TAG_ATT_BACKGROUND_IMG = 4,
	CSS_TAG_ATT_FONT_SIZE = 5,
	CSS_TAG_ATT_FONT_WEIGHT = 6,
	CSS_TAG_ATT_MARGIN = 7,
	CSS_TAG_ATT_PADDING = 8,
	CSS_TAG_ATT_FLOAT = 9,
	CSS_TAG_ATT_BORDER = 10,
};


const char css_att_type_list[11][24] =
{
	"background-color",
	"color",
	"width",
	"height",
	"background-img",
	"font-size",
	"font-weight",
	"margin",
	"padding",
	"float",
	"border"
};

const char css_tags_colors[10][10] =
{
	"red",
	"blue",
	"yellow",
	"green",
	"darkred",
	"darkblue",
	"darkyellow",
	"darkgreen",
	"orange",
	"pink"
};

const unsigned char css_colors_rgb[10 * 3] =
{
	255, 0, 0, 	// Red
	0, 0, 255, 	// Blue
	255, 255, 0, // Yellow
	0, 255, 0, 	// Green
	255, 0, 0, 	// Red
	0, 0, 255, 	// Blue
	255, 255, 0, // Yellow
	0, 255, 0, 	// Green
	255, 255, 0, // Yellow
	0, 255, 0 	// Green
};

/* It's in defines for now but we would need some way to put the CSS code under CDATA for easier processing.
 * (Aka converting HTML to XHTML compliant code, unless somehow we can make it play around it)
 *  */
#ifdef TIDY
uint write_cb(char *in, uint size, uint nmemb, TidyBuffer *out)
{
  uint r;
  r = size * nmemb;
  tidyBufAppend(out, in, r);
  return r;
}
 
/* Traverse the document tree */
void dumpNode(TidyDoc doc, TidyNode tnod, int indent)
{
  TidyNode child;
  for(child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
    ctmbstr name = tidyNodeGetName(child);
    if(name) {
      /* if it has a name, then it's an HTML tag ... */
      TidyAttr attr;
      printf("%*.*s%s ", indent, indent, "<", name);
      /* walk the attribute list */
      for(attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr) ) {
        printf(tidyAttrName(attr));
        tidyAttrValue(attr)?printf("=\"%s\" ",
                                   tidyAttrValue(attr)):printf(" ");
      }
      printf(">\n");
    }
    else {
      /* if it doesn't have a name, then it's probably text, cdata, etc... */
      TidyBuffer buf;
      tidyBufInit(&buf);
      tidyNodeGetText(doc, child, &buf);
      printf("%*.*s\n", indent, indent, buf.bp?(char *)buf.bp:"");
      tidyBufFree(&buf);
    }
    dumpNode(doc, child, indent + 4); /* recursive */
  }
}
#endif

void Load_Attribute_tags(unsigned char* att_val, size_t size_of_attribute)
{
	switch(last_set_type)
	{
		case IMG_TAG:
			switch(attb_set_type)
			{
				case IMG_SRC_TAG:
					if (img_toprint[number_of_img_print].filename)
					{
						free(img_toprint[number_of_img_print].filename);
						img_toprint[number_of_img_print].filename = NULL;
					}
					img_toprint[number_of_img_print].filename = malloc(size_of_attribute);
					snprintf(img_toprint[number_of_img_print].filename, size_of_attribute, "%s", att_val);
					
					#ifdef DEBUG_PARANOID
					printf("Image src %s\n", img_toprint[number_of_img_print].filename);
					#endif
							
					img_toprint[number_of_img_print].used = 1;
					
					status_toprint[number_of_files_print].type = STATUS_IMAGE_TYPE;
					status_toprint[number_of_files_print].number = number_of_img_print;
					
					number_of_img_print++;
					number_of_files_print++;
				break; 
			}
		break;
	}
}

void Load_Tags_Content(uint32_t tag_to_set, unsigned char* content_val, size_t size_of_content)
{
	switch(tag_to_set)
	{
		default:
		
		break;
		case HTML_TAG:
		break;
		case P_TAG:
			/* Don't consider the following as text */
			if ( !(size_of_content == 1 && content_val[0] == 10))
			{
				text_toprint[number_of_p_print].text_to_hold = malloc(size_of_content + 1);
				if (text_toprint[number_of_p_print].text_to_hold)
				{
					snprintf(text_toprint[number_of_p_print].text_to_hold, size_of_content + 1, "%s", (char *)content_val);
					text_toprint[number_of_p_print].type = 0;
					text_toprint[number_of_p_print].used = 1;
					#ifdef DEBUG_PARANOID
					printf("Text to print : %s\n\n", text_toprint[number_of_p_print].text_to_hold);
					#endif

					status_toprint[number_of_files_print].type = STATUS_TEXT_TYPE;
					status_toprint[number_of_files_print].number = number_of_p_print;

					#ifdef DEBUG_PARANOID
					printf("number_of_files_print %d, number_of_p_print %d,TYPE %d\n", number_of_files_print, number_of_p_print, status_toprint[number_of_files_print].type);
					#endif

					number_of_p_print++;
					number_of_files_print++;
				}
			}
		break;
		case TITLE_TAG:
			/* Reject any title text that is less than 3 characters and start with an empty space */
			if ( !(size_of_content < 3 && content_val[0] == 10))
			{
				if (title_html)
				{
					free(title_html);
					title_html = NULL;
				}
				
				/* Limit the title text size to 16 characters */
				if (size_of_content > 16)
				{
					size_of_content = 16;
				}
				title_html = malloc(size_of_content + 1);
				snprintf(title_html, size_of_content + 1, "%s", (char *)content_val);
			}
		break;
	}
}

void Set_Color_CSS(int type, int val)
{
	switch(type)
	{
		case 0:
			global_background_color = Return_RGB_color(css_colors_rgb[0+(val*3)], css_colors_rgb[1+(val*3)], css_colors_rgb[2+(val*3)]);
		break;
	}
}


/* These 2 functions should be one... TO FIX - gameblabla */
int Process_CSS_tag_to_ID(char* tag, size_t array_size)
{
	size_t i; char* pch;
	for(i=0;i<array_size;i++)
	{
		pch = strstr(tag, css_tags_type_list[i]);
		if (pch) return i;
	}
	return -1;
}

int Process_CSS_att_tag_to_ID(char* tag, size_t array_size)
{
	size_t i; char* pch;
	for(i=0;i<array_size;i++)
	{
		pch = strstr(tag, css_att_type_list[i]);
		if (pch) return i;
	}
	return -1;
}

void Process_CSS_tag(char* first_tag, char* second_tag, char* value)
{
	size_t i;
	const char* tag_to_process;
	char* pch;
	
	/* CSS styling */
	switch(Process_CSS_tag_to_ID(first_tag, ARRAY_SIZE(css_tags_type_list)))
	{
		case CSS_TAG_HTML:
		case CSS_TAG_BODY:
		case CSS_TAG_HEAD:
			/* Then we look through the supported tags */
			switch(Process_CSS_att_tag_to_ID(second_tag, ARRAY_SIZE(css_att_type_list)))
			{
				/* This makes sure that we only apply the background color for the main tags */
				case CSS_TAG_ATT_BACKGROUND_COLOR:
				for(i=0;i<ARRAY_SIZE(css_tags_colors);i++)
				{
					tag_to_process = css_tags_colors[i];
					pch = strstr(value, tag_to_process);
					if (pch)
					{
						Set_Color_CSS(0, i);
						// Break out of the loop
						break;
					}
				}
				break;
				default:
				
				break;
			}
		break;
		
	}

}

void Interpret_CSS(char* val, size_t size_v)
{
	size_t i;
	int CSS_read_status = 0;
	char* string;
	char tmp_retain[32];
	char att_retain[32][32];
	char val_att_retain[32][32];
	int go_through = 0;
	int att_counter = 0;
	string = val;
	
	memset(tmp_retain, 0, ARRAY_SIZE(tmp_retain));
	memset(att_retain, 0, 32 * 32);
	memset(val_att_retain, 0, 32 * 32);
	
	for(i=0;i<size_v;i++)
	{
		printf("string[i] %d, %c\n", string[i], string[i]);
		switch(CSS_read_status)
		{
			/* Empty space, trying to find a valid character that isn't a space or */
			case 0:
			if (!(string[i] != 10 && string[i] != 9 && string[i] != 32))
			{
				go_through = 0;
				att_counter = 0;
				memset(tmp_retain, 0, sizeof(tmp_retain));
				CSS_read_status = 1;
			}
			break;
			case 1:
				/* If we encounter {, then we can stop searching and begin searching inside */
				if (string[i] == '{')
				{
					printf("Processed tag is %s, now reading tags within it\n", tmp_retain);
					CSS_read_status = 2;
				}
				else
				{
					if (string[i] == 10 || string[i] == 9 || string[i] == 32)
					{
					}
					else
					{
						tmp_retain[go_through] = string[i];
						go_through++;
					}
				}
			break;
			case 2:
			if (!(string[i] != 10 && string[i] != 9))
			{
				go_through = 0;
				memset(att_retain[att_counter], 0, 32);
				CSS_read_status = 3;
			}
			break;
			case 3:
				/* If we encounter {, then we can stop searching and begin searching inside */
				if (string[i] == '}')
				{
					CSS_read_status = 0;
				}
				else if (string[i] == ':')
				{
					printf("Processed tag within %s is %s, now let's read its value\n", tmp_retain, att_retain[att_counter]);
					memset(val_att_retain[att_counter], 0, 32);
					go_through = 0;
					CSS_read_status = 4;
				}
				else
				{
					if (string[i] == 10 || string[i] == 9 || string[i] == 32)
					{
					}
					else
					{
						att_retain[att_counter][go_through] = string[i];
						go_through++;
					}
				}
			break;
			case 4:
				/* If we encounter {, then we can stop searching and begin searching inside */
				if (string[i] == '}')
				{
					CSS_read_status = 0;
				}
				else if (string[i] == ';')
				{
					printf("Processed value for tag %s within %s is %s, Done processing, going back to find more values.\n", tmp_retain, att_retain[att_counter], val_att_retain[att_counter]);
					
					Process_CSS_tag(tmp_retain, att_retain[att_counter], val_att_retain[att_counter]);
					att_counter++;
					CSS_read_status = 2;
				}
				else
				{
					if (string[i] == 10 || string[i] == 9 || string[i] == 32)
					{
					}
					else
					{
						val_att_retain[att_counter][go_through] = string[i];
						go_through++;
					}
				}
			break;
		}
	}
}

/* Traverse/Go through the XML/HTML trees and process the tags sequentially */
void traverse_dom_trees(xmlNode * a_node)
{
	size_t i;
	char* pch;
	
    xmlNode *cur_node = NULL;

    if(NULL == a_node)
    {
		#ifdef DEBUG_PARANOID
        printf("Invalid argument a_node %p\n", a_node);
        #endif
        return;
    }

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
    {
		switch(cur_node->type)
		{
			case XML_ELEMENT_NODE:
				/* Check for if current node should be exclude or not */
				//printf("Node type: Text, name: %s\n", cur_node->name);
				
				for(i=0;i<ARRAY_SIZE(available_tags);i++)
				{
					pch = strstr((char*)cur_node->name, available_tags[i]);
					if (pch)
					{
						last_set_type = i;
						#ifdef DEBUG_PARANOID
						printf("last_set_type %d\n", last_set_type);
						#endif
					}
				}
			break;
			case XML_TEXT_NODE:
				/* Process here text node, It is available in cpStr :TODO: */
				last_size_content = strlen((char *)cur_node->content);
				#ifdef DEBUG_PARANOID
				printf("node type: Text, node content: %s, content length %ld\n", (char *)cur_node->content, last_size_content);
				if (cur_node->parent->properties)
				{
					printf("Property %s, %ld\n", (char*)cur_node->parent->properties[0].name, last_size_content);
				}
				#endif
				
				Load_Tags_Content(last_set_type, cur_node->content, last_size_content);
			break;
			case XML_CDATA_SECTION_NODE:
				/* For CSS */
				last_size_content = strlen((char *)cur_node->content);
				#ifdef DEBUG
				printf("node type: CDATA, node content: %s, content length %ld\n", (char *)cur_node->content, last_size_content);
				#endif
				switch(last_set_type)
				{
					/* If this is a CSS tag, then let's go through it */
					case 4:
						Interpret_CSS((char *)cur_node->content, last_size_content);
					break;
				}
			break;
			case XML_ATTRIBUTE_NODE:
				/* Usually unused here (it's used below however) */
				#ifdef DEBUG_PARANOID
				printf("\n\nATTRIBUTE\n\n");
				printf("node type: Attribute, node content: %s\n", (char *)cur_node->parent->properties);
				#endif
			break;
			default:
				#ifdef DEBUG_PARANOID
				printf("\n\nUNUSED\n\n");
				printf("node type: DEFAULT, node content: %s, content length %ld\n", (char *)cur_node->content, last_size_content);
				#endif
			break;
		}
		
		/* Only try to process the attribute if there's any attribute to process */
		if (cur_node->properties)
		{
			uint32_t size_attb;
			xmlAttr* attribute = cur_node->properties;
			size_attb = 0;
			while(attribute && attribute->name && attribute->children)
			{
				xmlChar* value = xmlNodeListGetString(cur_node->doc, attribute->children, 1);
				#ifdef DEBUG_PARANOID
				printf ("Atribute %s: %s\n",attribute->name, value);
				#endif
			  
				for(i=0;i<ARRAY_SIZE(img_tags);i++)
				{
					pch = strstr((char*)attribute->name, img_tags[i]);
					if (pch)
					{
						attb_set_type = i;
						size_attb = strlen((char*)value) + 1;
						#ifdef DEBUG_PARANOID
						printf("last_set_type %d\n", last_set_type);
						#endif
						
						/* This is where we actually handle the attribute specific tags */
						Load_Attribute_tags(value, size_attb);
						
						/* Break out of loop */
						break;
					}
				}
				
				xmlFree(value);
				attribute = attribute->next;
			}
		}
		
		//last_set_type = 0;
		attb_set_type = 0;

        traverse_dom_trees(cur_node->children);
    }
}

void Load_All_Images(void)
{
	int i;
	
	#ifdef DEBUG
	printf("Images found %d\n", number_of_img_print);
	#endif
	
	for(i=0;i<number_of_img_print;i++)
	{
		#ifdef DEBUG
		printf("Loading image %s\n", img_toprint[i].filename);
		#endif
		img_toprint[i].texture = Load_Image_game(img_toprint[i].filename);
		if (!img_toprint[i].texture)
		{
			#ifdef DEBUG
			printf("Failed to load %s in array %d\n", img_toprint[i].filename, i);
			#endif
			img_toprint[i].used = 0;
		}
		else
		{
			#ifdef DEBUG
			printf("Loaded Image %s in array %d\n", img_toprint[i].filename, i);
			#endif
			img_toprint[i].used = 1;
		}
	}
	
	//return 1;
}

int main (int argc, char *argv[]) 
{
	int i = 0;
	int text_y = 0;
	int quit = 0;
		
    htmlDocPtr doc;
    xmlNode *roo_element = NULL;

    if (argc != 2)  
    {
        printf("\nInvalid argument\n");
        return(1);
    }
	
	if (!Init_Video(video_width, video_height, bitdepth))
	{
		printf("Failed to create window, exit\n");
		return 0;
	}
	
	/* Make sure to point to the real directory of the file we try to open */
	char* path =  dirname(realpath(argv[1], NULL));
	chdir(path);
	
	/* Will be changeable with global styling and CSS */
	global_background_color = Return_RGB_color(255, 255, 255);

    /* Macro to check API for match with the DLL we are using */
    LIBXML_TEST_VERSION    

	/* Parse the HTML code */
    doc = htmlReadFile(argv[1], NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
    if (doc == NULL) 
    {
        fprintf(stderr, "HTML file not parsed successfully.\n");
        return 0;
    }

    roo_element = xmlDocGetRootElement(doc);

    if (roo_element == NULL) 
    {
        fprintf(stderr, "Empty document..\n");
        xmlFreeDoc(doc);
        return 0;
    }

    traverse_dom_trees(roo_element);
    
    /* We now load the images, set the title... here */
    
    Set_title(title_html, NULL);
    Load_All_Images();
	
	while(!quit)
	{
		text_y = 0;
		
		/* We go through the stuff that we have to draw in memory */
		for(i=0;i<number_of_files_print;i++)
		{
			switch(status_toprint[i].type)
			{
				case STATUS_IMAGE_TYPE:
					if (img_toprint[status_toprint[i].number].used)
					{
						Display_image(img_toprint[i].texture, 4, text_y);
						text_y += Get_Image_Height(img_toprint[i].texture);
					}
					//
				break;
				case STATUS_TEXT_TYPE:
					Display_Font(internal_fonts[0], text_toprint[status_toprint[i].number].text_to_hold, 4, text_y, 32, 32, 32, 255, 0);
					text_y += 12;
				break;
			}
		}

		Flip_video(global_background_color);
		
		Poll_Controls();
		if (keys_status[11] == 1) quit = 1;
	}
	
	//Unload_Image(tex);
	Quit_video();
}
