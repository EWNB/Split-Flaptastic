# Elliot Baptist 28 Feb 2019

import argparse
from collections import namedtuple
from enum import Enum
import os
from pickletools import float8
from typing import Any, Dict, List, NamedTuple, Tuple, Union
import cairo
from math import floor
import tempfile
import subprocess


# --- Types/Classes ---

class TextFlap(NamedTuple):
    '''Represents a text character to be drawn and turned into a flap'''
    char: str
    font: str
    scale: float
    offset: float = 0.0

    def surface(self,
                flap_w: float,
                flap_halfh: float) -> cairo.RecordingSurface:
        rect = cairo.Rectangle(0,
                               0,
                               int(mm_to_pt(flap_w)),
                               int(mm_to_pt(flap_halfh * 2)))
        new_char_sfc = cairo.RecordingSurface(cairo.Content.COLOR_ALPHA, rect)
        ctx = cairo.Context(new_char_sfc)
        ctx.scale(mm_to_pt(1), mm_to_pt(1))
        ctx.select_font_face(self.font)
        font_h = int(flap_halfh * 1.5)
        ctx.set_font_size(int(font_h * self.scale))
        ctx.move_to(0, int(flap_halfh + font_h / 2))
        ctx.show_text(self.char)
        x, y, width, height = new_char_sfc.ink_extents()
        ctx.save()
        ctx.set_operator(cairo.Operator.CLEAR)
        ctx.paint()
        ctx.restore()
        ctx.move_to(int((flap_w - pt_to_mm(width)) / 2 - pt_to_mm(x)),
                    int(flap_halfh + font_h / 2))
        ctx.show_text(self.char)
        return new_char_sfc


class ImageScaleType(Enum):
    MARGIN = 0
    SCALE = 1


class ImageScale(NamedTuple):
    type: ImageScaleType
    value: float


class ImageFlap(NamedTuple):
    '''Represents a pre-rendered image to be turned into a flap'''
    path: os.PathLike  # must be a PNG
    scale: ImageScale
    offset: float = 0.0

    def surface(self,
                flap_w: float,
                flap_halfh: float) -> cairo.PDFSurface:
        i_sfc = cairo.ImageSurface.create_from_png(self.path)
        pdf_sfc = cairo.PDFSurface(None,
                                   mm_to_pt(flap_w),
                                   mm_to_pt(flap_halfh * 2))
        ctx = cairo.Context(pdf_sfc)
        if self.scale.type is ImageScaleType.MARGIN:
            h_scale = (mm_to_pt(flap_w - self.scale.value)
                       / i_sfc.get_width())
            v_scale = (mm_to_pt(flap_halfh * 2 - self.scale.value)
                       / i_sfc.get_height())
            _scale = min(h_scale, v_scale)
        elif self.scale.type is ImageScaleType.SCALE:
            _scale = self.scale.value
        else:
            raise NotImplementedError('Unexpected ImageScaleType',
                                      self.scale.type)
        h_margin = mm_to_pt(flap_w) - i_sfc.get_width() * _scale
        v_margin = mm_to_pt(flap_halfh * 2) - i_sfc.get_height() * _scale
        ctx.translate(h_margin / 2, v_margin / 2)
        ctx.scale(_scale, _scale)
        ctx.set_source_surface(i_sfc)
        ctx.paint()
        return pdf_sfc


Flap = Union[TextFlap, ImageFlap]

# --- Constants ---

PAGE_SIZE_A4 = 210, 297  # mm
PAGE_SIZE_A3 = 297, 420  # mm


# --- Helper Functions ---

def mm_to_pt(mm: float) -> float:
    return mm*2.8346456692913


def pt_to_mm(pt: float) -> float:
    return pt/2.8346456692913


def calc_num_flaps(page_sz: float, margin_sz: float, flap_sz: float) -> int:
    return int((page_sz - margin_sz * 2) // flap_sz)


def cross(ctx: cairo.Context,
          span: float,
          tblr=['up', 'down', 'left', 'right']) -> None:
    if 'up' in tblr:
        ctx.rel_move_to(0, -span / 2)
        ctx.rel_line_to(0,  span / 2)
    if 'down' in tblr:
        ctx.rel_move_to(0,  span / 2)
        ctx.rel_line_to(0, -span / 2)
    if 'left' in tblr:
        ctx.rel_move_to(-span / 2, 0)
        ctx.rel_line_to(span / 2, 0)
    if 'right' in tblr:
        ctx.rel_move_to(span / 2, 0)
        ctx.rel_line_to(-span / 2, 0)


def flap_border(ctx: cairo.Context, width: float, half_h: float) -> None:
    ctx.rel_line_to(width, 0)
    ctx.rel_line_to(0,  2 * half_h)
    ctx.rel_line_to(-width, 0)
    ctx.rel_line_to(0, -2 * half_h)


def flap_cut_marks(ctx: cairo.Context,
                   width: float,
                   half_h: float,
                   mark_len: float) -> None:
    cross(ctx, mark_len, ['down', 'right'])
    ctx.rel_move_to(width, 0)
    cross(ctx, mark_len, ['down', 'left'])
    ctx.rel_move_to(0,  2 * half_h)
    cross(ctx, mark_len, ['up', 'left'])
    ctx.rel_move_to(-width, 0)
    cross(ctx, mark_len, ['up', 'right'])
    ctx.rel_move_to(0, -2 * half_h)


# --- Main Function ---

def flaps(filename: str,
          page_size: Tuple[float, float],
          flap_size: Tuple[float, float],
          flap_list: List[Flap]) -> None:
    # -- Config --
    # flap
    flap_w, flap_h = flap_size
    flap_halfh = flap_h / 2
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
    page_w, page_h = page_size
    # printer
    margin_w = 5
    margin_h = 5

    # -- Page orientation and flap count --
    # Calculate flap count in landscape orientation
    n_x = calc_num_flaps(page_w, margin_w, flap_halfh * 2)
    n_y = calc_num_flaps(page_h, margin_h, flap_w)
    n_flaps_l = n_x * n_y
    # Calculate flap count in portrait orientation
    n_x = calc_num_flaps(page_w, margin_w, flap_w)
    n_y = calc_num_flaps(page_h, margin_h, flap_halfh * 2)
    n_flaps_p = n_x * n_y
    if n_flaps_l > n_flaps_p:
        # Rotate page to landscape for better flap yeild
        page_w, page_h = page_h, page_w
        margin_w, margin_h = margin_h, margin_w

    # Calculate grid
    n_x = calc_num_flaps(page_w, margin_w, flap_w)
    n_y = calc_num_flaps(page_h, margin_h, flap_halfh * 2)
    x_start = ((page_w - margin_w * 2) % flap_w) / 2 + margin_w
    y_start = ((page_h - margin_h * 2) % (flap_halfh * 2)) / 2 + margin_h
    x_end = x_start + (n_x - 1) * flap_w
    y_end = y_start + (n_y - 1) * flap_halfh * 2
    n_flaps_per_page = n_x * n_y

    # -- Standardise characters using cairo surfaces --
    char_sfcs = [flap.surface(flap_w, flap_halfh) for flap in flap_list]

    # -- Prepare characters for drawing --
    # Crop to half characters
    n_chars = len(char_sfcs)
    hchar_tops = []
    hchar_bots = []
    for cs in char_sfcs:
        for n in range(2):
            crop_sfc = cs.create_similar(cairo.Content.COLOR_ALPHA,
                                         int(mm_to_pt(flap_w)),
                                         int(mm_to_pt(flap_halfh)))
            ctx = cairo.Context(crop_sfc)
            ctx.set_source_surface(cs, 0, mm_to_pt(-flap_halfh) if n else 0)
            ctx.paint()
            if n == 0:
                hchar_tops.append(crop_sfc)
            else:
                hchar_bots.append(crop_sfc)

    # Add space
    include_space = True
    if include_space:
        hchar_tops.append(None)
        hchar_bots.append(None)
        n_chars += 1

    # Shift character bottoms to make flaps align
    hchar_bots.append(hchar_bots.pop(0))

    # -- Prepare to draw --
    # Calculate page and flap count
    pages = (n_chars + n_flaps_per_page - 1) // n_flaps_per_page
    n_flaps = pages * n_flaps_per_page

    # Create output surface
    with cairo.PDFSurface(filename, mm_to_pt(page_w), mm_to_pt(page_h)) as sfc:
        ctx = cairo.Context(sfc)
        y = y_start
        x = x_start
        # -- Draw each flap --
        for idx in range(n_flaps):
            ctx.scale(mm_to_pt(1), mm_to_pt(1))
            # Border
            ctx.move_to(x, y)
            ctx.set_source_rgba(0, 0, 0, 0.1)
            ctx.set_line_width(cut_stroke)
            flap_border(ctx, flap_w, flap_halfh)
            ctx.stroke()
            # Edge marks
            ctx.set_source_rgba(0, 0, 0, 0.5)
            ctx.set_line_width(cut_stroke)
            if x == x_start and y == y_start:
                ctx.move_to(x, 0)
                ctx.rel_line_to(0, cut_edge_len)
                ctx.move_to(0, y)
                ctx.rel_line_to(cut_edge_len, 0)
            if y == y_start:
                ctx.move_to(x + flap_w, 0)
                ctx.rel_line_to(0, cut_edge_len)
            if x == x_start:
                ctx.move_to(0, y + flap_halfh * 2)
                ctx.rel_line_to(cut_edge_len, 0)
            if x == x_end and y == y_end:
                ctx.move_to(x + flap_w, page_h)
                ctx.rel_line_to(0, -cut_edge_len)
                ctx.move_to(page_w, y + flap_halfh * 2)
                ctx.rel_line_to(-cut_edge_len, 0)
            if y == y_end:
                ctx.move_to(x, page_h)
                ctx.rel_line_to(0, -cut_edge_len)
            if x == x_end:
                ctx.move_to(page_w, y)
                ctx.rel_line_to(-cut_edge_len, 0)
            ctx.stroke()
            # Cut marks
            ctx.move_to(x, y)
            ctx.set_source_rgba(0, 0, 0, 0.5)
            ctx.set_line_width(cut_stroke)
            flap_cut_marks(ctx, flap_w, flap_halfh, cut_cross_len)
            ctx.stroke()
            # Fold line
            ctx.move_to(x + flap_w / 2, y + flap_halfh)
            ctx.set_source_rgba(1, 0, 0, 0.3)
            ctx.set_line_width(fold_stroke)
            ctx.set_dash([fold_dash_len, fold_dash_len], 0)
            fold_width = (
                (floor((flap_w - 2 * fold_inset) / (fold_dash_len * 2))
                 * fold_dash_len * 2)
                + fold_dash_len
            )
            ctx.rel_move_to(-fold_width / 2, 0)
            ctx.rel_line_to(fold_width, 0)
            ctx.stroke()
            ctx.set_dash([])
            # Shaft marks
            ctx.move_to(x + flap_w / 2, y + flap_halfh)
            ctx.set_source_rgba(0, 0, 1, 0.3)
            ctx.set_line_width(axle_stroke)
            ctx.rel_move_to(-flap_w / 2, -axle_offset)
            ctx.rel_line_to(axle_mark_len, 0)
            ctx.rel_move_to(flap_w-axle_mark_len * 2, 0)
            ctx.rel_line_to(axle_mark_len, 0)
            ctx.stroke()
            # Flap number
            # ctx.select_font_face('Consolas')
            ctx.set_source_rgba(0, 0, 0, 0.3)
            ctx.set_font_size(3)
            ctx.move_to(x + 1, y + 3)
            ctx.show_text(str(idx))
            # Character
            ctx.scale(1 / mm_to_pt(1), 1 / mm_to_pt(1))
            if idx < n_chars:
                ctx.translate(mm_to_pt(x), mm_to_pt(y))
                for n, hchar in enumerate([hchar_tops[idx], hchar_bots[idx]]):
                    if hchar != None:
                        char_width = flap_w
                        char_hheight = flap_halfh
                        h_offset = 0
                        v_offset = char_hheight if n else 0
                        ctx.translate(mm_to_pt(h_offset), mm_to_pt(v_offset))
                        ctx.set_source_surface(hchar)
                        ctx.paint()
                        ctx.translate(-mm_to_pt(h_offset), -mm_to_pt(v_offset))
                ctx.translate(-mm_to_pt(x), -mm_to_pt(y))
            # Update position
            if y == y_end and x == x_end and idx < n_flaps-1:
                ctx.show_page()
                y = y_start
                x = x_start
            elif x == x_end:
                y += flap_halfh*2
                x = x_start
            else:
                x += flap_w

    print(f'Flaps drawn to "{filename}"')


# --- Entrypoint ---

if __name__ == '__main__':

    flap_list = [
        TextFlap('0', 'Boogaloo', 1.40),
        TextFlap('1', 'Boogaloo', 1.40),
        TextFlap('2', 'Boogaloo', 1.40),
        TextFlap('3', 'Boogaloo', 1.40),
        TextFlap('4', 'Boogaloo', 1.40),
        TextFlap('5', 'Boogaloo', 1.40),
        TextFlap('6', 'Boogaloo', 1.40),
        TextFlap('7', 'Boogaloo', 1.40),
        TextFlap('8', 'Boogaloo', 1.40),
        TextFlap('9', 'Boogaloo', 1.40),
        TextFlap('-', 'Boogaloo', 1.40),
        TextFlap('|', 'Boogaloo', 1.40),
        TextFlap('+', 'Boogaloo', 1.40),
        TextFlap('x', 'Boogaloo', 1.40),
        TextFlap('?', 'Boogaloo', 1.40)
    ]

    flaps('pyflap.pdf', PAGE_SIZE_A4, (50, 75), flap_list)


# # Get list of objects in Inkscape file
# input_inkscape_file = os.path.join('Flaps','32_char.svg')
# objects = subprocess.check_output('inkscape -z -S {}'.format(input_inkscape_file), shell=True).decode('utf-8')

# # Remove undesired objects and keep only the object id
# object_ids = [l.split(',')[0] for l in objects.splitlines() if l.startswith('path')]

# # Export inkscape objects to temporary folder.
# # TODO: use `-shell`` option to speed up as per
# # https://stackoverflow.com/questions/11457931/running-an-interactive-command-from-within-python#13458449
# export_dpi = 200
# with tempfile.TemporaryDirectory() as tmpdir:
#     for val in object_ids:
#         filename = os.path.join(tmpdir, val + '.png')
#         cmd = ['inkscape', '-z', '-j', '-i', val,
#                '-d', export_dpi, '-e', filename, input_inkscape_file]
#         subprocess.check_call(cmd)
