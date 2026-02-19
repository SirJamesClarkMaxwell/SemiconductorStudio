from manim import *
from random import randint


class Fractions(Scene):
    def construct(self):
        E = (
            VGroup(
                *[
                    MathTex(rf"\frac{{{randint(1,999)}}}{{{randint(1,999)}}}")
                    for _ in range(50)
                ]
            )
            .arrange_in_grid(rows=10, cols=5)
            .scale_to_fit_height(10)
        )
        self.add(E)
