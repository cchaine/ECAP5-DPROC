BUILDDIR=build

docs: arch.tex

arch.tex: builddir
	pdflatex -halt-on-error -output-directory ${BUILDDIR} docs/arch/main.tex 
	cp build/main.pdf docs/arch.pdf

builddir:
	mkdir -p ${BUILDDIR}
