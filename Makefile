BUILDDIR=build

docs: arch.tex

arch.tex: builddir
	cd docs/ && pdflatex --interaction=nonstopmode -halt-on-error -output-directory ../${BUILDDIR} arch/main.tex 
	cp build/main.pdf docs/arch.pdf

synth:
	yosys -s config/synth.conf

builddir:
	mkdir -p ${BUILDDIR}
