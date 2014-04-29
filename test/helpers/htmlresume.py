__author__ = 'thurley'


class Htmlwriter:
    # file object

    def __init__(self, filename):
        self.f = open(filename, 'w')

    def make_document(self, head, body):
        self.f.write("<!DOCTYPE html>\n<html>")
        self.f.write("<header>"
                        + head
                    + "</header>")
        self.f.write("<body>"
                     + body
                     + "</body>")
        self.f.write("</html>")






def add_tag(tag, text):
    return "<" + tag + ">" + text + "</" + tag + ">"

def add_cell(text):
    return add_tag("td", text)

def add_row(content):
    return add_tag("tr", content)

def produce_html_rows(rowlist):
    htmlrows = ""
    for row in rowlist:
        rowcontent = ""
        for cell in row:
            rowcontent += add_cell(cell)
        htmlrows += add_row(rowcontent)
    return htmlrows

class Htmltable:

    def __init__(self):
        self.head = [[]]
        self.content = [[]]

    def add_row(self, cells):
        self.content.append(cells)

    def add_header(self, cells):
        self.head.append(cells)

    def make(self):
        head = add_tag("thead", produce_html_rows(self.head))
        content = add_tag("tbody", produce_html_rows(self.content))
        return add_tag("table", head + content)
