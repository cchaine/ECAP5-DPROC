BUILDDIR=build

docs: arch.tex

arch.tex: builddir
	pdflatex -halt-on-error -output-directory ${BUILDDIR} docs/arch/main.tex 

builddir:
	mkdir -p ${BUILDDIR}
