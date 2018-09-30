import os, shutil
import logging
import re

source_dir = "./"
build_dir = "./build/"
script_tag = re.compile(r"<script\s*src=\"([\w.\/]+)\"><\/script>")

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
    rm_content_of_dir(build_dir)
else:
    logging.info("Creating build dir...")
    os.makedirs(build_dir)

logging.info("done.")

def process_html_file(path):
    content = open(path, 'r').read()
    match = script_tag.findall(content)
    print(match)
    if not match:
        logging.info('No script tag found to process')
    else:
        logging.info('Found: %s', match.groups())

#    print (content)

logging.info("Processing html files...")
html_dir = os.path.join(source_dir, '')
for filename in os.listdir(html_dir):
    if filename.endswith(".html"):
        path = os.path.join(html_dir, filename)
        logging.info("Processing %s", path)
        process_html_file(path)

