import _windows_dll_fix

from matplotlib import mathtext

p = mathtext.MathTextParser('bitmap')
print(p.parse('$\\sum$'))
