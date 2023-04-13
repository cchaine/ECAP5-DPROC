BUILDDIR=build

docs: arch.tex

arch.tex: builddir
	pdflatex -halt-on-error -output-directory ${BUILDDIR} docs/arch/arch.tex 

builddir:
	mkdir -p ${BUILDDIR}
