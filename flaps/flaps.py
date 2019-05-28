# Elliot Baptist 28 Feb 2019

import argparse
import os
import cairo
import math
import tempfile
import subprocess

def mm_to_pt(mm):
  return mm*2.8346456692913
def pt_to_mm(pt):
  return pt/2.8346456692913

def cross(ctx, span, tblr=['up','down','left','right']):
  if 'up' in tblr:
    ctx.rel_move_to(0, -span/2)
    ctx.rel_line_to(0, span/2)
  if 'down' in tblr:
    ctx.rel_move_to(0, span/2)
    ctx.rel_line_to(0, -span/2)
  if 'left' in tblr:
    ctx.rel_move_to(-span/2, 0)
    ctx.rel_line_to(span/2, 0)
  if 'right' in tblr:
    ctx.rel_move_to(span/2, 0)
    ctx.rel_line_to(-span/2, 0)

def flap_boarder(ctx, width, half_height):
  ctx.rel_line_to(width, 0)
  ctx.rel_line_to(0, 2*half_height)
  ctx.rel_line_to(-width, 0)
  ctx.rel_line_to(0, -2*half_height)

def flap_cut_marks(ctx, width, half_height, mark_len):
  cross(ctx, mark_len, ['down','right'])
  ctx.rel_move_to(width, 0)
  cross(ctx, mark_len, ['down','left'])
  ctx.rel_move_to(0, 2*half_height)
  cross(ctx, mark_len, ['up','left'])
  ctx.rel_move_to(-width, 0)
  cross(ctx, mark_len, ['up','right'])
  ctx.rel_move_to(0, -2*half_height)


# Config
# flap
flap_hheight = 37.5
flap_width = 50
# grid
grid_width = 6
grid_height = 6
# lines
cut_stroke = 0.1
cut_edge_len = 12
cut_cross_len = 10
fold_stroke = 0.1
fold_dash_len = 4
fold_inset = 4
axle_stroke = 0.1
axle_mark_len = 1
axle_offset = 4
# page
A4 = 210, 297
A3 = 297, 420
page_width, page_height = A3
# printer
margin_width = 5
margin_height = 5

# Test portrait orientation
num_x_a = (page_width-margin_width*2)//flap_width
num_y_a = (page_height-margin_height*2)//(flap_hheight*2)
num_flaps_a = num_x_a * num_y_a
# Test landscape orientation
num_x_b = (page_width-margin_width*2)//(flap_hheight*2)
num_y_b = (page_height-margin_height*2)//flap_width
num_flaps_b = num_x_b * num_y_b
# Rotate page for best flap yeild
if num_flaps_a < num_flaps_b:
  page_width, page_height = page_height, page_width
  margin_width, margin_height = margin_height, margin_width

# Calculate grid
num_x = int((page_width-margin_width*2)//flap_width)
num_y = int((page_height-margin_height*2)//(flap_hheight*2))
x_start = ((page_width-margin_width*2)%flap_width)/2 + margin_width
y_start = ((page_height-margin_height*2)%(flap_hheight*2))/2 + margin_height
x_end = x_start + (num_x-1)*flap_width
y_end = y_start + (num_y-1)*flap_hheight*2
flaps_per_page = num_x * num_y

# TODO make list with a dict for each flap, including inkscape id or character (and font), and centering information (offset or center)

# Get list of objects in Inkscape file
input_inkscape_file = os.path.join('Flaps','32_char.svg')
objects = subprocess.check_output('inkscape -z -S {}'.format(input_inkscape_file), shell=True).decode('utf-8')
print(objects)
# print(type(objects))
assert type(objects) == str

# Remove undesired objects and keep only the object id
object_ids = [l.split(',')[0] for l in objects.splitlines() if l.startswith('path')]
print(object_ids)

# Create temporary folder for holding inscape exports
# tmpdir = tempfile.TemporaryDirectory()

# Export inkscape objects to temporary folder
# TODO use -shell option to speed up https://stackoverflow.com/questions/11457931/running-an-interactive-command-from-within-python#13458449
char_surfaces = []
# normal_dpi = 96
export_dpi = 200
# char_scale = mm_to_pt(1)*export_dpi/normal_dpi
pt_per_in = 72
pt_per_px = pt_per_in/export_dpi
# max_width = 0
# max_height = 0
# for val in object_ids:
#   filename = os.path.join('Flaps', 'tmp', val+'.png')
#   # os.system('inkscape -z -j -i {} -d {} -e {} {}'.format(val, export_dpi, filename, input_inkscape_file)) #tmpdir.name
#   img_surface = cairo.ImageSurface.create_from_png(filename)
#   # max_width = max(max_width, new_char_surface.get_width())
#   # max_height = max(max_height, new_char_surface.get_height())
#   pdf_surface = cairo.PDFSurface(None, mm_to_pt(flap_width), mm_to_pt(flap_hheight*2))
#   ctx = cairo.Context(pdf_surface)
  # char_horz_margin = mm_to_pt(flap_width) - img_surface.get_width()*pt_per_px
  # char_vert_margin = mm_to_pt(flap_hheight*2) - img_surface.get_height()*pt_per_px
  # ctx.translate(char_horz_margin/2, char_vert_margin/2)
  # ctx.scale(pt_per_px, pt_per_px)
  # ctx.set_source_surface(img_surface)
  # ctx.paint()
  # char_surfaces.append(pdf_surface)

# Text characters
font_size_correction = {None:1.32, 'Boogaloo':1.40}
font = 'Boogaloo'
font_height = int(flap_hheight*1.5)
for ch in list('ABCDEFGHIJKLMNOPQRSTUVWXYZ?!&/,'):
  # new_char_surface = cairo.PSSurface(None, int(mm_to_pt(flap_width)), int(mm_to_pt(flap_hheight*2)))
  new_char_surface = cairo.RecordingSurface(cairo.Content.COLOR_ALPHA, cairo.Rectangle(0,0,int(mm_to_pt(flap_width)), int(mm_to_pt(flap_hheight*2))))
  ctx = cairo.Context(new_char_surface)
  ctx.scale(mm_to_pt(1), mm_to_pt(1))
  ctx.select_font_face(font)
  ctx.set_font_size(int(font_height*font_size_correction[font]))
  ctx.move_to(0, int(flap_hheight+font_height/2))
  ctx.show_text(ch)
  x,y,width,height = new_char_surface.ink_extents()
  rect = new_char_surface.get_extents()
  ctx.save()
  ctx.set_operator(cairo.Operator.CLEAR)
  ctx.paint()
  ctx.restore()
  ctx.move_to(int((flap_width-pt_to_mm(width))/2-pt_to_mm(x)), int(flap_hheight+font_height/2))
  ctx.show_text(ch)
  char_surfaces.append(new_char_surface)

# Crop to half characters
num_chars = len(char_surfaces)
hchar_tops = []
hchar_bots = []
for cs in char_surfaces:
  for n in range(2):
    # crop_surf = cairo.ImageSurface(cairo.Format.ARGB32, cs.get_width(), cs.get_height()//2)
    # ctx.set_source_surface(cs, 0, (-cs.get_height()/2) if n else 0)
    crop_surf = cs.create_similar(cairo.Content.COLOR_ALPHA, int(mm_to_pt(flap_width)), int(mm_to_pt(flap_hheight)))
    ctx = cairo.Context(crop_surf)
    ctx.set_source_surface(cs, 0, mm_to_pt(-flap_hheight) if n else 0)
    ctx.paint()
    if n == 0:
      hchar_tops.append(crop_surf)
    else:
      hchar_bots.append(crop_surf)

# del char_surfaces

# Add space
include_space = True
if include_space:
  # space_surf = cairo.ImageSurface(cairo.Format.ARGB32, 1, 1)
  hchar_tops.append(None)
  hchar_bots.append(None)
  num_chars += 1

# Shift character bottoms to make flaps align
hchar_bots.append(hchar_bots.pop(0))

# Create output surface
with cairo.PDFSurface(os.path.join('Flaps','pyflap.pdf'), mm_to_pt(page_width), mm_to_pt(page_height)) as surface:
  ctx = cairo.Context(surface)

  # Work in mm
  # ctx.scale(mm_to_pt(1), mm_to_pt(1))

  # ctx.select_font_face('Boogaloo')
  # ctx.set_font_size(80)
  # ctx.move_to(4, int(flap_hheight*2)-4)
  # ctx.show_text('A')
  # ctx.set_source_surface(char_surfaces[0])
  # ctx.paint()

  # Draw
  y = y_start
  x = x_start
  pages = (num_chars+flaps_per_page-1)//flaps_per_page
  flaps_total = pages*flaps_per_page
  for i in range(flaps_total):
    ctx.scale(mm_to_pt(1), mm_to_pt(1))
    # Boarder
    ctx.move_to(x, y)
    ctx.set_source_rgba(0,0,0, 0.1)
    ctx.set_line_width(cut_stroke)
    flap_boarder(ctx, flap_width, flap_hheight)
    ctx.stroke()
    # Edge marks
    ctx.set_source_rgba(0,0,0, 0.5)
    ctx.set_line_width(cut_stroke)
    if x == x_start and y == y_start:
      ctx.move_to(x, 0)
      ctx.rel_line_to(0, cut_edge_len)
      ctx.move_to(0, y)
      ctx.rel_line_to(cut_edge_len, 0)
    if y == y_start:
      ctx.move_to(x+flap_width, 0)
      ctx.rel_line_to(0, cut_edge_len)
    if x == x_start:
      ctx.move_to(0, y+flap_hheight*2)
      ctx.rel_line_to(cut_edge_len, 0)
    if x == x_end and y == y_end:
      ctx.move_to(x+flap_width, page_height)
      ctx.rel_line_to(0, -cut_edge_len)
      ctx.move_to(page_width, y+flap_hheight*2)
      ctx.rel_line_to(-cut_edge_len, 0)
    if y == y_end:
      ctx.move_to(x, page_height)
      ctx.rel_line_to(0, -cut_edge_len)
    if x == x_end:
      ctx.move_to(page_width, y)
      ctx.rel_line_to(-cut_edge_len, 0)
    ctx.stroke()
    # Cut marks
    ctx.move_to(x, y)
    ctx.set_source_rgba(0,0,0, 0.5)
    ctx.set_line_width(cut_stroke)
    flap_cut_marks(ctx, flap_width, flap_hheight, cut_cross_len)
    ctx.stroke()
    # Fold line
    ctx.move_to(x+flap_width/2, y+flap_hheight)
    ctx.set_source_rgba(1,0,0, 0.3)
    ctx.set_line_width(fold_stroke)
    ctx.set_dash([fold_dash_len,fold_dash_len], 0)
    fold_width = math.floor((flap_width-2*fold_inset)/(fold_dash_len*2))*fold_dash_len*2+fold_dash_len
    ctx.rel_move_to(-fold_width/2, 0)
    ctx.rel_line_to(fold_width, 0)
    ctx.stroke()
    ctx.set_dash([])
    # Shaft marks
    ctx.move_to(x+flap_width/2, y+flap_hheight)
    ctx.set_source_rgba(0,0,1, 0.3)
    ctx.set_line_width(axle_stroke)
    ctx.rel_move_to(-flap_width/2, -axle_offset)
    ctx.rel_line_to(axle_mark_len, 0)
    ctx.rel_move_to(flap_width-axle_mark_len*2, 0)
    ctx.rel_line_to(axle_mark_len, 0)
    ctx.stroke()
    # Flap number
    # ctx.select_font_face('Consolas')
    ctx.set_source_rgba(0,0,0, 0.3)
    ctx.set_font_size(3)
    ctx.move_to(x+1, y+3)
    ctx.show_text(str(i))
    # Character
    ctx.scale(1/mm_to_pt(1), 1/mm_to_pt(1))
    if i < num_chars:
      ctx.translate(mm_to_pt(x), mm_to_pt(y))
      # ctx.scale(1/char_scale,1/char_scale)
      for n,hchar in enumerate([hchar_tops[i], hchar_bots[i]]):
        if hchar != None:
          char_width = flap_width #hchar.get_width()
          char_hheight = flap_hheight #hchar.get_height()
          # h_offset = (char_scale*flap_width-char_width)/2
          h_offset = 0 #(flap_width-char_width)/2
          # v_offset =  char_scale*flap_hheight - (0 if n else char_hheight)
          v_offset =  char_hheight if n else 0
          ctx.translate(mm_to_pt(h_offset), mm_to_pt(v_offset))
          ctx.set_source_surface(hchar)
          ctx.paint()
          ctx.translate(-mm_to_pt(h_offset), -mm_to_pt(v_offset))
      # ctx.scale(char_scale,char_scale)
      ctx.translate(-mm_to_pt(x), -mm_to_pt(y))
    # Update position
    if y == y_end and x == x_end and i < flaps_total-1:
      ctx.show_page()
      y = y_start
      x = x_start
    elif x == x_end:
      y += flap_hheight*2
      x = x_start
    else:
      x += flap_width

print('Done')
