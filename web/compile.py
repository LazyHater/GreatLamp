import os, shutil
import logging
import re
import json
from subprocess import check_output

source_dir = "./"
build_dir = "./build/"
script_tag = re.compile(r"<script\s*src=\"([\w.\/]+)\"><\/script>")
script_tag_replace = r"<script\s*src=\"{script_name}\"><\/script>"

link_tag = re.compile(r"<link href=\"([\w.\/]+)\" rel=\"stylesheet\" type=\"text/css\">")
link_tag_replace = r"<link href=\"{link_name}\" rel=\"stylesheet\" type=\"text/css\">"

html_file_template =  """
// Html.h

#ifndef HTML_H
#define HTML_H

{}

#endif
"""

deviceinfo_file_template =  """
// Deviceinfo.h

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

{}
const int DEVICEINFO_PAYLOAD_SIZE = {};

#endif
"""

h_line_template =  "const char {}[] PROGMEM = {};"
device_line_template =  "const char {}[] PROGMEM = {};"
mimify_command = "html-minifier --minify-css --minify-js --remove-comments --conservative-collapse --collapse-whitespace  --remove-tag-whitespace --remove-attribute-quotes  {}"
# mimify_command = "html-minifier --minify-css --minify-js --remove-comments   {}"
#regex = r"<script\s*src=\"([\w.\/]+)\"><\/script>"
#script_tag = re.compile(regex)

logging.basicConfig(level=logging.DEBUG)

def rm_content_of_dir(dname):
    for the_file in os.listdir(dname):
        file_path = os.path.join(dname, the_file)
        try:
            if os.path.isfile(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path): shutil.rmtree(file_path)
        except Exception as e:
            logging,error(e)

if (os.path.exists(build_dir)):
    logging.info("Build dir already exists, removing content...")
    shutil.rmtree(build_dir)
    os.makedirs(build_dir)
else:
    logging.info("Creating build dir...")
    os.makedirs(build_dir)

logging.info("done.")

def embed_js(content, script_name):
    script = open(script_name, 'r').read()
    script_tag_replace_re = re.compile(script_tag_replace.format(script_name=script_name))
    match = script_tag_replace_re.findall(content)
    logging.info("Replacing tags %s", match)
    return script_tag_replace_re.sub("<script>\n{}\n</script>".format(script), content)
    
def embed_css(content, link_name):
    link = open(link_name, 'r').read()
    link_tag_replace_re = re.compile(link_tag_replace.format(link_name=link_name))
    match = link_tag_replace_re.findall(content)
    logging.info("Replacing tags %s", match)
    return link_tag_replace_re.sub("<style>\n{}\n</style>".format(link), content)

def handle_script_tags(content):
    match = script_tag.findall(content)
    if not match:
        logging.info('No script tag found to process')
        return content
    else:
        logging.info('Found: %s', match)

    for script_name in match:
        content = embed_js(content, script_name)
    return content

def handle_link_tags(content):
    match = link_tag.findall(content)
    if not match:
        logging.info('No link tag found to process')
        return content
    else:
        logging.info('Found: %s', match)

    for link_name in match:
        content = embed_css(content, link_name)
    return content

def process_html_file(path):
    with open(path, 'r') as f:
        content = f.read()

    logging.info('Processing script tags...')
    content = handle_script_tags(content)

    logging.info('Processing link tags...')
    content = handle_link_tags(content)

    fname = path.split('/')[-1]

    out_path = os.path.join(build_dir, fname)
    with open(out_path, 'w') as f:
        f.write(content)
        
    logging.info('Mimifing')
    print(mimify_command.format(out_path))
    mimified = check_output(mimify_command.format(out_path).split())
    with open(out_path + '.min', 'w') as f:
        f.write(mimified)

    logging.info('Saving as C string')
    string_for_printing = json.dumps(mimified)
    with open(out_path + '.min.c', 'w') as f:
        f.write(string_for_printing)

    logging.info("Processing %s done.", path)
    

logging.info("Processing html files...")
html_min_dir = source_dir
for filename in os.listdir(html_min_dir):
    if filename.endswith(".html"):
        path = os.path.join(html_min_dir, filename)
        logging.info("Processing %s", path)
        process_html_file(path)

def escape_to_c_str(path, fname):
    logging.info("Processing %s", path)
    with open(path, 'r') as f:
        content = f.read()
    return (h_line_template.format(fname, json.dumps(content)), len(content))

logging.info("Generating Html.h file...")

files = [ os.path.join(build_dir, fname) for fname in os.listdir(build_dir) if fname.endswith(".html.min")]

html_file_src = os.path.join(build_dir, "Html.h")
html_file_dst = os.path.join(build_dir, "../../esp_code/Html.h")
with open(html_file_src, 'w') as f:
    f.write(html_file_template.format("\n".join( [escape_to_c_str(file, file.split('/')[-1].split('.')[0].upper()+"_HTML")[0] for file in files])))

logging.info("Copying Html.h to esp dir...")
shutil.copyfile(html_file_src, html_file_dst)

logging.info("Done")
   

with open("../esp_code/Deviceinfo.h", 'w') as fout:
    content, size = escape_to_c_str("deviceinfo.json", "DEVICEINFO_PAYLOAD")
    devinfo = deviceinfo_file_template.format(content, size).replace('char', 'uint8_t')
    fout.write(devinfo)


