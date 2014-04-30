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

def add_tag_w_attr(tag, attributes, text):
    return "<" + tag + " " + attributes + ">" + text + "</" + tag + ">"

def add_cell(text):
    return add_tag("td", text)

def add_cell_w_attr(attr, text):
    return add_tag_w_attr("td", attr, text)

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

def produce_html_rows_w_classes(rowlist):
    htmlrows = ""
    for row in rowlist:
        rowcontent = ""
        for cell in row:
            rowcontent += add_cell_w_attr("class=\"" + cell[0] + "\"", cell[1])
        htmlrows += add_row(rowcontent)
    return htmlrows

class Htmltable:

    def __init__(self):
        self.head = [[]]
        self.content = [[]]
    def add_row_w_cell_classes(self, cells):
        self.content.append(cells)

    def add_header_w_cell_classes(self, cells):
        self.head.append(cells)

    def make(self):
        head = add_tag("thead", produce_html_rows_w_classes(self.head))
        content = add_tag("tbody", produce_html_rows_w_classes(self.content))
        return add_tag_w_attr("table", "border=\"0\" cellpadding=\"2\" cellspacing=\"2\" style=\"font-family:arial;width:854px\"", head + content)
