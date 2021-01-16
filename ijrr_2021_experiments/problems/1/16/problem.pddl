(define (problem ijrr_16)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street8 street23 - street
		building17 building23 building24 building32 - building
	)
	(:init
		(atLocation survivor0 street8)
		(atLocation survivor1 building17)
		(onFire building23)
		(onFire building32)
		(underRubble street23)
		(underRubble building24)
		(needsRepair building23)
		(needsRepair building24)
		(needsRepair building32)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street23))
		(not (needsRepair building23))
		(not (needsRepair building24))
		(not (needsRepair building32))
	))
	(:metric minimize total-time)
)
