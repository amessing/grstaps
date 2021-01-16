(define (problem ijrr_2)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street6 street45 - street
		building16 building17 building24 building33 - building
	)
	(:init
		(atLocation survivor0 street6)
		(atLocation survivor1 building17)
		(onFire building24)
		(onFire building33)
		(underRubble street45)
		(underRubble building16)
		(needsRepair building16)
		(needsRepair building24)
		(needsRepair building33)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street45))
		(not (needsRepair building16))
		(not (needsRepair building24))
		(not (needsRepair building33))
	))
	(:metric minimize total-time)
)
