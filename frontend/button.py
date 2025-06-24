import pygame as p
p.font.init()

BLUE = (0, 0, 255)
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)


class Button:
    def __init__(self, x, y, length, height, text="", text_size=30, main_color=WHITE, edge_color=BLACK, edge=1):
        self.coords = (x, y)
        self.length = int(length)
        self.height = int(height)
        self.text = text
        self.text_size = int(text_size)
        self.text_color = BLACK
        self.main_color = main_color
        self.edge_color = edge_color
        self.edge = edge
        self.edge_color = BLACK
        self.highlight_color = main_color
        self.highlight = False
        self.main_surface = p.Surface((length, height))
        self.font = p.font.SysFont("timesnewroman", self.text_size)
        self.text_surface = self.font.render(self.text, False, self.text_color)
        self.tag = None
        self.pos = None
        self.active = True
        self.sprite = None

    def check_if_pressed(self, x, y):
        if not self.active:
            return False

        if self.coords[0] < x < self.coords[0] + self.length:
            if self.coords[1] < y < self.coords[1] + self.height:
                return True
        return False

    def set_text(self, text=None, text_size=None):
        if text is not None:
            self.text = text
            self.text_surface = self.font.render(self.text, False, self.text_color)
        if text_size is not None:
            text_size = int(text_size)
            self.text_size = text_size
            self.font = p.font.SysFont("timesnewroman", self.text_size)
            self.text_surface = self.font.render(self.text, False, self.text_color)

    def update_highlight(self):
        mouse_coords = p.mouse.get_pos()
        self.highlight = False
        if self.coords[0] < mouse_coords[0] < self.coords[0] + self.length:
            if self.coords[1] < mouse_coords[1] < self.coords[1] + self.height:
                self.highlight = True

    def write_text(self, surface):
        text_surface_size = self.text_surface.get_rect()
        self.font.render(self.text, False, self.text_color)
        button_center = (self.coords[0] + self.length / 2, self.coords[1] + self.height / 2)
        text_x = button_center[0] - (text_surface_size[2] / 2)
        text_y = button_center[1] - (text_surface_size[3] / 2)
        surface.blit(self.text_surface, (text_x, text_y))

    def draw(self, surface):
        if self.highlight:
            color = self.highlight_color
        else:
            color = self.main_color

        self.main_surface.fill(color)
        surface.blit(self.main_surface, (self.coords[0], self.coords[1]))
        p.draw.rect(surface, self.edge_color, (self.coords[0], self.coords[1], self.length, self.height), self.edge)

        if self.sprite != None:
            surface.blit(self.sprite, (self.coords[0], self.coords[1]))
        self.write_text(surface)

    def set_sprite(self, sprite):
        self.sprite = sprite
